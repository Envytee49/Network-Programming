#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char DB_HOST[16];       
    char DB_PORT[6];        
    char DB_NAME[16];        
    char DB_USER[10];      
    char DB_PASS[6];        
    int PORT;         
    char SECRET_KEY[32];    
} Config;

extern Config *config;  

int load_config(const char *filename, Config *config);
void init_config();

#endif 
