import json
import socket
import struct

class Client:
    def __init__(self):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect("/tmp/ryzen-adj-daemon.sock")

    def _send_req(self, what):
        payload = json.dumps(what).encode('utf-8')
        payload_len = struct.pack('@I', len(payload))
        self.sock.sendall(payload_len)
        self.sock.sendall(payload)

    def _recv_resp(self):
        payload_len = self.sock.recv(4)
        payload_len = struct.unpack('@I', payload_len)[0]
        payload = self.sock.recv(payload_len)
        resp = json.loads(payload.decode('utf-8'))
        if resp["status"] != 0:
            raise Exception(f"Error: {resp['msg']} ({resp['status']})")
        return resp["data"]

    def close(self):
        self.sock.close()

    def get_info(self):
        self._send_req({"req": "get_info"})
        return self._recv_resp()

    def get_metrics_table(self):
        self._send_req({"req": "get_metrics_table"})
        return self._recv_resp()

    def set_slow_limit(self, value):
        self._send_req({"req": "set_slow_limit", "value": value})
        self._recv_resp()

    def set_fast_limit(self, value):
        self._send_req({"req": "set_fast_limit", "value": value})
        self._recv_resp()

    def set_stapm_limit(self, value):
        self._send_req({"req": "set_stapm_limit", "value": value})
        self._recv_resp()

    def exit(self):
        self._send_req({"req": "exit"})
        self._recv_resp()


def main():
    client = Client()
    try:
        print(client.get_info())
        old_table = client.get_metrics_table()
        print("stapm limit: ", old_table["stapm_limit"])
        print("slow limit: ", old_table["ppt_limit_slow"])
        print("fast limit: ", old_table["ppt_limit_fast"])
        client.set_slow_limit(5.0)
        client.set_fast_limit(10.0)
        client.set_stapm_limit(8.0)
        new_table = client.get_metrics_table()
        print("After setting new limits:")
        print("stapm limit: ", new_table["stapm_limit"])
        print("slow limit: ", new_table["ppt_limit_slow"])
        print("fast limit: ", new_table["ppt_limit_fast"])
        client.exit()
    except Exception as e:
        print("Error communicating with daemon:", e)
    finally:
        client.close()


if __name__ == "__main__":
    main()
