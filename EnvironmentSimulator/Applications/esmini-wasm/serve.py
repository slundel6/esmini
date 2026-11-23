#!/usr/bin/env python3

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from ipaddress import ip_address
from pathlib import Path
import socket
from urllib.parse import urlsplit


APP_DIR = Path(__file__).resolve().parent
REPO_ROOT = APP_DIR.parents[2]
RESOURCES = {
    "/resources/xosc/wasm-example.xosc": REPO_ROOT / "resources/xosc/wasm-example.xosc",
    "/resources/xodr/wasm-example.xodr": REPO_ROOT / "resources/xodr/wasm-example.xodr",
    "/resources/xosc/Catalogs/VehicleCatalog/VehicleCatalog.xosc": (
        REPO_ROOT / "resources/xosc/Catalogs/Vehicles/VehicleCatalog.xosc"
    ),
}


class Handler(SimpleHTTPRequestHandler):
    def translate_path(self, path):
        resource = RESOURCES.get(urlsplit(path).path)
        if resource is not None:
            return str(resource)
        return super().translate_path(path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Serve the esmini WASM app locally.")
    parser.add_argument("--bind", type=ip_address, default="127.0.0.1", help="IP address to bind")
    parser.add_argument("--port", type=int, default=8080, help="port to bind")
    args = parser.parse_args()
    if not 0 <= args.port <= 65535:
        parser.error("port must be between 0 and 65535")

    class Server(ThreadingHTTPServer):
        address_family = socket.AF_INET6 if args.bind.version == 6 else socket.AF_INET

    handler = partial(Handler, directory=str(APP_DIR))
    with Server((str(args.bind), args.port), handler) as server:
        host = f"[{args.bind}]" if args.bind.version == 6 else str(args.bind)
        print(f"Serving http://{host}:{server.server_port}/", flush=True)
        server.serve_forever()
