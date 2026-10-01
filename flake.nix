{
  description = "Keycard — smartcard authentication module for Logos Basecamp";

  inputs = {
    # builder 0.3.1: universal interface (impl header -> generated glue + LIDL contract).
    logos-module-builder.url = "github:logos-co/logos-module-builder/0.3.1";

    # Follow the builder's nixpkgs to avoid Qt ABI mismatches
    nixpkgs.follows = "logos-module-builder/nixpkgs";

    # keycard-qt source (non-flake) — pinned to our fork (xAlisher/keycard-qt)
    # This commit adds Schnorr/BIP340 TLV-unwrap + ECDSA DER fix (not merged upstream).
    # URL uses xAlisher fork explicitly; do NOT switch to status-im/keycard-qt until
    # both fixes land upstream (track: status-im/keycard-qt#96, #132 equivalents).
    # lgx CLI: re-adds each variant after stripping pcsclite so the manifest hashes stay correct.
    logos-package.url = "github:logos-co/logos-package/c25a1167578aef5cbd9a9b6f822ffe2ae4fd6a89";

    keycard-qt-src = {
      url = "github:xAlisher/keycard-qt/5cd0b0d22659fd8564c749787fc86c1374b00223";
      flake = false;
    };
  };

  outputs = inputs@{ logos-module-builder, nixpkgs, keycard-qt-src, logos-package, ... }:
    let
      # Build keycard-qt as a pre-compiled static library for each target system.
      # The builder's mkExternalLib machinery stages it into lib/ before cmake runs;
      # LogosModule.cmake's EXTERNAL_LIBS finds libkeycard-qt.a + include/keycard-qt/*.h.
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];

      keycardQtPackages = builtins.listToAttrs (map (system:
        let
          pkgs = import nixpkgs { inherit system; };
          drv = pkgs.stdenv.mkDerivation {
            pname = "keycard-qt";
            version = "0.1.0";
            src = keycard-qt-src;

            nativeBuildInputs = [ pkgs.cmake pkgs.ninja pkgs.pkg-config ];
            buildInputs = [
              pkgs.qt6.qtbase
              pkgs.openssl
              pkgs.pcsclite
            ];
            # macOS: nixpkgs' pcsclite puts winscard.h and pcsclite.h in include/PCSC, and winscard.h
            # includes <pcsclite.h> with angle brackets — add that dir so it resolves (Linux finds it
            # via pkg-config already). Without a pcscd on macOS the module loads with no reader.
            env.NIX_CFLAGS_COMPILE = pkgs.lib.optionalString pkgs.stdenv.isDarwin
              "-I${pkgs.lib.getDev pkgs.pcsclite}/include/PCSC";

            # Library only — no app wrapping, no tests, no examples
            dontWrapQtApps = true;
            cmakeFlags = [
              "-GNinja"
              "-DCMAKE_BUILD_TYPE=Release"
              "-DBUILD_TESTING=OFF"
              "-DBUILD_EXAMPLES=OFF"
            ];

            # Remove cmake package config files — only lib + headers are needed by
            # the builder's EXTERNAL_LIBS mechanism.  Keeping them would cause the
            # module's cmake setup-hook to fail trying to patch read-only store files.
            postInstall = ''
              rm -rf $out/lib/cmake
            '';
          };
        in { name = system; value = { default = drv; }; }
      ) systems);

      # Strip pcsclite from the lgx-portable output. Qt's transitive closure
      # includes pcsclite-2.3.0 so the bundler always copies it in; but system
      # pcscd is typically 2.0.3 and the version mismatch breaks the IPC socket
      # handshake. After stripping, the bundler sets RPATH to $ORIGIN only, so
      # libpcsclite.so.1 is looked up via standard system paths (2.0.3) at runtime.
      stripPcscliteFromLgx = system: lgxDrv:
        let pkgs = import nixpkgs { inherit system; };
            lgx = "${logos-package.packages.${system}.all}/bin/lgx";
        in pkgs.runCommand lgxDrv.name { nativeBuildInputs = [ pkgs.jq pkgs.gnutar pkgs.gzip ]; } ''
          mkdir -p $out
          LGX=$(ls ${lgxDrv}/*.lgx)
          BNAME=$(basename "$LGX")
          cp "$LGX" "$out/$BNAME"; chmod u+w "$out/$BNAME"
          tmpdir=$(mktemp -d)
          tar -xzf "$LGX" -C "$tmpdir"
          # Re-add every variant without libpcsclite: `lgx add` replaces the variant AND recomputes
          # the manifest hashes (deleting files from the tarball alone leaves them stale, and
          # `lgx verify` / Basecamp 0.3 then reject the package). assets/ is left untouched.
          for vdir in "$tmpdir"/variants/*/; do
            v=$(basename "$vdir")
            main=$(jq -r --arg v "$v" '.main[$v]' "$tmpdir/manifest.json")
            find "$vdir" -name "libpcsclite.so*" -delete
            ${lgx} add "$out/$BNAME" --variant "$v" --files "$vdir" --main "$main" -y
          done
          if tar -tzf "$out/$BNAME" | grep -q 'libpcsclite.so'; then
            echo "ERROR: libpcsclite still present" >&2; exit 1
          fi
          ${lgx} verify "$out/$BNAME" | grep -v -i 'unsigned' | grep -i -E 'mismatch|error' && exit 1 || true
          rm -rf "$tmpdir"
        '';

      baseModule = logos-module-builder.lib.mkLogosModule {
        src = ./.;
        configFile = ./metadata.json;
        flakeInputs = inputs;

        # keycard-qt resolved per-system above.
        # The builder copies $drv/lib/* into lib/ and $drv/include/* into lib/
        # so that LogosModule.cmake EXTERNAL_LIBS finds libkeycard-qt.a + headers.
        externalLibInputs = {
          "keycard-qt" = { packages = keycardQtPackages; };
        };
      };

    in baseModule // {
      # Override lgx-portable to strip pcsclite from the final LGX tarball
      packages = builtins.mapAttrs (system: sysPkgs:
        sysPkgs // {
          lgx-portable = stripPcscliteFromLgx system sysPkgs.lgx-portable;
        }
      ) baseModule.packages;
    };
}
