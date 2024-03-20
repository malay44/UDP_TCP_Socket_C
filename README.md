# UDP Video Streaming

- Uses pthreads for handling multiple clients concurrently.

## Commands 
- `ffmpeg -i sample_video.mp4 -f mpegts streamable_output.mp4` for converting a video to a streamable format.

- `gcc server_udp.c -o server_udp.out && ./server_udp.out ` for running the server.

- `gcc client_udp.c -o client_udp.out && ./client_udp.out 0.0.0.0 recv.mp4` for running the client.