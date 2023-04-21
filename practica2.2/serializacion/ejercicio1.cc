#include "Serializable.h"

#include <iostream>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define MAX_NAME 50
#define MAX_DATA_SIZE 1024
/*
El comando od ermite mostrar el contenido de un archivo en formato octal, decimal, hexadecimal o ASCII. 
Por defecto, od muestra el contenido del archivo en formato octal.
Con el flag -sa muestra el contenido en formato ASCII

0000000     123     987   27728   31073   29285   20319   17742       0
          { nul   [ etx   P   l   a   y   e   r   _   O   N   E nul nul

La serialización de un objeto consiste en convertir su contenido en una secuencia de bytes 
que puedan ser almacenados en un archivo o transmitidos a través de una red. 

Por lo tanto, la salida de od puede ser utilizada para verificar que la serialización 
de un objeto se ha realizado correctamente y que los bytes que se han almacenado 
o transmitido corresponden al contenido original del objeto.
*/
class Jugador: public Serializable
{
public:
    Jugador(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y)
    {
        strncpy(name, _n, 80);
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        alloc_data(MAX_NAME + 2*sizeof(int16_t));

        char* tmp = _data;

        memcpy(tmp, &x, sizeof(int16_t));
        tmp += sizeof(int16_t);

        memcpy(tmp, &y, sizeof(int16_t));
        tmp += sizeof(int16_t);

        memcpy(tmp, name, MAX_NAME);
        tmp += MAX_NAME;
    }

    int from_bin(char * data)
    {
        char* tmp = data;

        memcpy(&x, tmp, sizeof(int16_t));
        tmp += sizeof(int16_t);

        memcpy(&y, tmp, sizeof(int16_t));
        tmp += sizeof(int16_t);

        memcpy(name, tmp, MAX_NAME);
        tmp += MAX_NAME;
        
        // Verificar que el nombre termine por fin de cadena
        if (name[MAX_NAME - 1] != '\0') {
            return -1;
        }

        return 0;
    }

    std::string getNombre(){
        return name;
    }

    int16_t getPuntos(){
        return x;
    }

    int16_t getVidas(){
        return y;
    }

private:
    char name[80];

    int16_t x; // puntos
    int16_t y; // vidas
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("Player_ONE", 123, 987);

    one_w.to_bin();
    char* serialized = one_w.data();

    // 2. Escribir la serialización en un fichero
    int fd = open("jugador.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1){
        perror("open error\n");
        return 1;
    }
    ssize_t bytes_written = write(fd, serialized, one_w.size());
    if (bytes_written == -1) {
        perror("write error\n");
        close(fd);
        return 1;
    }else if (bytes_written == 0){
        perror("zero bytes written\n");
        close(fd);
        return 1;
    }
    close(fd);

    // 3. Leer el fichero
    std::ifstream file_read;
    file_read.open("jugador.txt");
    std::string content((std::istreambuf_iterator<char>(file_read)), std::istreambuf_iterator<char>());
    file_read.close();

    // 4. "Deserializar" en one_r
    int success = one_r.from_bin(&content[0]);

    // 5. Mostrar el contenido de one_r
    if (success == 0){
        std::cout << "Nombre: " << one_r.getNombre() << std::endl;
        std::cout << "Puntos: " << one_r.getPuntos() << std::endl;
        std::cout << "Vidas: " << one_r.getVidas() << std::endl;
    }else{
        std::cout << "Error serializando\n";
    }

    return 0;
}

