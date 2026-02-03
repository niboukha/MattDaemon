#ifndef CONFIG_HPP
#define CONFIG_HPP

class Config {
public:
    static Config& getInstance();
    
    int getPort() const;
    void setPort(int port);
    
private:
    Config();
    int port;
};

#endif // CONFIG_HPP
