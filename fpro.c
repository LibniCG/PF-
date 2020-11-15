#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <curses.h>
#include <sys/stat.h>
#include <sys/mman.h>

/*** Declaración de variables globales ****/
int fs;
int fd;
char *map;

typedef struct datos { //Datos de la imagen
    short int *sector; //Tamaño del sector
    int spc; //Sectores por cluster
    short int *sr; //Sectores reservados
    int copias; //Número de copias del FAT
    short int *re; //Entradas del directorio raíz
    int *sd; //Número de sectores del disco
    int tam; //Tamaño del FAT
    char ev[11]; //Etiqueta volumen
    char id[5]; //ID Sistema
} INFOIMG;

/*** Declaración de funciones ****/
char *mapFile(char *filePath);
int getNext(int cluster, int base);
void pruebas();
void getInfo();
void openF (char *filename);

int main(int argc, char *argv[]){
    openF(argv[1]);
}

char *mapFile(char *filePath){

    fd = open(filePath, O_RDWR);
    if (fd == -1){
        perror("Error");
        return(NULL);
    }
    
    struct stat st;
    fstat(fd, &st);
    fs = st.st_size;

    char *mapF = mmap(0, fs+200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapF == MAP_FAILED) {
        close(fd);
        perror("Error mapeando el archivo");
        return(NULL);
    }
    return mapF;
}

int getNext(int cluster, int base) {
	// Para FAT12
	int offset = cluster + cluster/2;
	int flag = cluster % 2; // Nos dice si en la parte baja o alta

	unsigned char b1,b2;
      //printf("%x", map);
	b1 = map[base+offset];
	//printf("%02x:",b1);
	b2 = map[base+offset+1];
	//printf("%02x\n",b2);
	int result = b1 | b2<<8; // Los bits mas significativos van al final

	if(flag) {
		result >>= 4;
	}
	else {
		result &= 0xfff;
	}

	//printf("%04x\n",result);
  
	return result;
}

void pruebas(){
    int d;
    for(int i = 0; i<10; i++){
        d = getNext (i,0x200);
        printf("%03x %d\n",d,d);
    }
}

void getInfo(){

    INFOIMG info;

    printf("\n\n\t****** BPB (BIOS Parameter Block) ******\n");

    info.sector = (short int *)&map[11]; //Tamaño sector
    printf("\tTamaño sector                %d \n", *info.sector); ///int aux = info.sector;
    
    info.spc =  map[13]; //Sectores por cluster
    printf("\tSectores por cluster         %d \n", info.spc);
    
    info.sr = (short int *)&map[14]; //Sectores reservados
    printf("\tSectores por reservados      %d \n", *info.sr);
    
    info.copias = map[16]; //Num de copias del FAT
    printf("\tNumero de copias del FAT     %d \n", info.copias);
    
    info.re = (short int *)&map[17]; //Entadas del directorio raíz
    printf("\tEntradas directorio raiz     %d \n", *info.re);
    
    info.sd = (int *)&map[32]; //Num sectores del disco
    printf("\tNumero sectores del disco    %d \n", *info.sd);

    info.tam = map[22]; //Tamaño del FAT
    printf("\tTamaño del FAT               %d \n", info.tam);
    
    strcpy(info.ev, &map[43]); //Etiqueta de volumen
    printf("\tEtiqueta de volumen          %s \n", info.ev);

    strncpy(info.id, &map[0x36],5); //ID del Sistema
    printf("\tId Sistema                   %s \n\n", info.id);

    int dirRaiz = (*info.sr + (info.tam * info.copias)) * (*info.sector);
    printf("\tDirectorio Raíz              0x%04x\n", dirRaiz);   

    int datos = dirRaiz + ((*info.re) * 32);
    printf("\tInicio de Datos              0x%04x\n\n", datos); 
}

void openF (char *filename){

    map = mapFile(filename);
    
    if (map ==NULL){
      exit(EXIT_FAILURE);
    }
    
    //pruebas();
    getInfo();
 
    
    if (munmap(map,fs) == -1){
        perror("Error");
    }
    
    close(fs);
}