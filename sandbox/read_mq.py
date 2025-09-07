import posix_ipc
try:
    posix_ipc.unlink_message_queue('/test_queue')
except Exception:
    pass
import struct
import time

STRUCT_FORMAT = '<ddd100s'
MSG_SIZE = struct.calcsize(STRUCT_FORMAT)
QUEUE_NAME = '/test_queue'

print(f"[Python] Waiting for message queue '{QUEUE_NAME}' to appear...")

mq = None
for attempt in range(50):
    try:
        mq = posix_ipc.MessageQueue(QUEUE_NAME, posix_ipc.O_RDONLY)
        print(f"[Python] Message queue opened successfully on attempt {attempt + 1}.")
        break
    except posix_ipc.ExistentialError:
        print(f"[Python] Attempt {attempt + 1}: Queue not found, retrying...")
        time.sleep(0.1)
    except Exception as e:
        print(f"[Python] Attempt {attempt + 1}: Unexpected error: {e}")
        time.sleep(0.1)

if mq is None:
    print("[Python] Message queue not found after waiting.")
    exit(1)

try:
    print("[Python] Waiting for messages...")
    while True:
        msg, _ = mq.receive(MSG_SIZE)
        max_cpu, max_mem, max_pids, container_id = struct.unpack(STRUCT_FORMAT, msg)
        print("Raw container_id bytes:", container_id)
        print("Container:", container_id.decode('utf-8', errors='replace').rstrip('\x00'),
              "| Max CPU:", max_cpu,
              "| Max Mem:", max_mem,
              "| Max PIDs:", max_pids)
except KeyboardInterrupt:
    print("[Python] Interrupted by user.")
finally:
    mq.close()
    print("[Python] Message queue closed.")