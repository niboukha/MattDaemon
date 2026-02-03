#include "Config.hpp"

Config::Config() : port(8080) {}

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

int Config::getPort() const {
    return port;
}

void Config::setPort(int port) {
    this->port = port;
}
