{
  description = "gbridge application with local greybus";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    defaultPackage.${system} = pkgs.stdenv.mkDerivation rec {
      pname = "gbridge";
      version = "0.1.0";
      # Use builtins.path to hash the current directory
      src = pkgs.lib.cleanSource (builtins.path { path = ./.; });
        
      dontStrip = true;
      enableDebugging = true;

      nativeBuildInputs = [
        pkgs.autoreconfHook
        pkgs.pkg-config
      ];
      buildInputs = [
        pkgs.libnl
        pkgs.bluez
        pkgs.avahi
      ];
      configureFlags = [
        "--enable-tcpip"
        "--enable-netlink"
        "--disable-bluetooth"
        "--disable-gbsim"
      ];
      #patchPhase = ''
      #  sed -i 's/manifest->bundle_node/bundle_node/g' protocols/manifest.c
      #  sed -i 's/bundle->cport_node/cport_node/g' protocols/manifest.c
      #'';
      preConfigure = ''
        export CFLAGS="-g -I$(pwd)/include $CFLAGS"
        export CPPFLAGS="$CFLAGS"
      '';
      preBuild = ''
        export NIX_CFLAGS_COMPILE="$NIX_CFLAGS_COMPILE $CFLAGS $(pkg-config --cflags libnl-3.0 libnl-genl-3.0) $(pkg-config --cflags bluez) $(pkg-config --cflags avahi-client)"
        export NIX_LDFLAGS="$NIX_LDFLAGS $(pkg-config --libs avahi-client)"
      '';
      meta = with pkgs.lib; {
        description = "gbridge application with local greybus";
        homepage = "https://github.com/example/gbridge";
        license = licenses.mit;
        platforms = platforms.linux;
      };
    };
  };
}

