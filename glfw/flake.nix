{
  description = "Mousy Click - GLFW Potion";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          binutils
          gnumake
          pkg-config
          glfw3
          glew
        ];
      };
    };
}
