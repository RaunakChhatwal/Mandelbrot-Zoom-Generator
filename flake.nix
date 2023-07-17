{
  description = "Arbitrary-precision mandelbrot set generator";

  inputs.nixpkgs.url = "nixpkgs/nixos-23.05";

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs { 
      inherit system; 
    };
  in {
    defaultPackage.${system} = pkgs.stdenv.mkDerivation {
      name = "raunak-mandelbrot-${pkgs.lib.version}-src";
      
      src = ./.;

      buildInputs = with pkgs; [
        clang_16
        libpng
        gmp
      ];

      CXXCOMPILER = "${pkgs.clang_16.out}/bin/clang++";
      CXXFLAGS = "-std=c++20 -Wall -Wextra -Wno-sign-compare -O3";
      LDFLAGS = "-L${pkgs.libpng.out}/lib -lpng -L${pkgs.gmpxx.out}/lib -lgmp -lgmpxx";

      buildPhase = ''
        make
      '';

      installPhase = ''
        mkdir -p $out/bin
        cp main $out/bin/
      '';

    };
  };
}
