{
  description = "Syzygui - Cross Platform Vulkan Based GUI Framework";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system: let
      version = "0.0.1";
      
      pkgs = import nixpkgs {
        inherit system;
      };

      deps = (with pkgs; [
        uthash
      ]);
      
      propDeps = (with pkgs; [
        fuse3
      ]) ++ (pkgs.lib.optionals pkgs.stdenv.isDarwin (with pkgs; [
      ]));

      flatTmpFuse = pkgs.stdenv.mkDerivation {
        pname = "flatTmpFuse";
        inherit version;
        src = ./.;

        nativeBuildInputs = with pkgs; [
          meson
          ninja
          pkg-config
        ];

        buildInputs = deps ++ (with pkgs; [
        ]);

        propagatedBuildInputs = propDeps ++ (with pkgs; [
        ]);
      };
      
    in {
        packages = {
          default = flatTmpFuse;
        };
        apps = rec {
          default = flake-utils.lib.mkApp { drv = self.packages.${system}.default; };
        };

        devShells.default = pkgs.mkShell {
          C_INCLUDE_PATH = "./include";
          packages = deps ++ propDeps ++ (with pkgs; [
            clang-tools
            meson
          ]);
        };
      }
    );
}
