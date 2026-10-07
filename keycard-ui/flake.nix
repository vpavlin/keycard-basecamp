{
  description = "Keycard UI — QML debug panel and auth harness for the Keycard module";

  inputs = {
    # port/0.3: builder 0.3.1. Basecamp 0.3 needs every dependency to publish an interface
    # contract, so the keycard core is an explicit input (the universal port, 1.1.0).
    logos-module-builder.url = "github:logos-co/logos-module-builder/0.3.1";
    keycard.url = "github:vpavlin/keycard-basecamp/5e89fdf6ad226d3b0710d057b840f19f6d0b80e7";
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosQmlModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;
    };
}
