#pragma once

namespace flow{

enum class node_type{
    none = 0,
    image_local,
    camera,
    image_crop,
    image_blob,
    modbus_out,
    tcp_out,
    http_out,
    image_save
};
}