#pragma once

bool serial_port_open(const char* ip, int port);

bool serial_port_read_line(char* out_buf, int out_buf_size);

void serial_port_close();
