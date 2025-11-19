//https://github.com/Pfaff2005/Bot_Info1R5---Manuel-Pfaff.git
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

struct memory {
	char *response;
	size_t size;
};

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp)
{
	size_t realsize = nmemb;
	struct memory *mem = clientp;
	
	char *ptr = realloc(mem->response, mem->size + realsize + 1);
	if (!ptr)
		return 0;  /* out of memory */
	
	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), data, realsize);
	mem->size += realsize;
	mem->response[mem->size] = 0;
	
	return realsize;
}

int main(void)
{
	char token[150];
	long offset = 0;
	char url[600];
	char first_name[64] = "";
	int update_id;
	char chat_id[11];
	char mensaje[256] = "";
	char fecha[20];
	FILE *log;
	bool nuevo_msj;
	
	// Lectura del token desde archivo
	FILE *tok;
	tok = fopen("token.txt", "r");
	if (tok == NULL) {
		printf("No se abrio el archivo del token\n");
		exit(1);
	} else {
		fscanf(tok, "%s", token);
		fclose(tok);
	}
	
	CURLcode res;
	CURL *curl = curl_easy_init();
	struct memory chunk = {0};
	
	while (1) {
		mensaje[0] = '\0';
		first_name[0] = '\0';
		struct memory chunk = {0}; // para reiniciar la rta
		
		if (curl) {
			sprintf(url, "https://api.telegram.org/bot%s/getUpdates?offset=%ld", token, offset);
			
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
			
			res = curl_easy_perform(curl);
			
			if (res != 0)
				printf("Error Codigo: %d\n", res);
			
			printf("%s\n", chunk.response);
			
			// buscar update_id
			char *ptr = strstr(chunk.response, "\"update_id\":");
			if (ptr) {
				sscanf(ptr, "\"update_id\":%d", &update_id);
				offset = update_id + 1;
				nuevo_msj = 1;
			} else {
				printf("No hay mensajes nuevos\n");
				nuevo_msj = 0;
			}
			
			// buscar chat_id
			ptr = strstr(chunk.response, "\"chat\":{\"id\":");
			if (ptr) {
				sscanf(ptr, "\"chat\":{\"id\":%s", chat_id);
			}
			
			// buscar nombre
			ptr = strstr(chunk.response, "\"first_name\":\"");
			if (ptr) {
				sscanf(ptr, "\"first_name\":\"%63[^\"]\"", first_name);
			}
			
			// buscar texto del mensaje
			ptr = strstr(chunk.response, "\"text\":\"");
			if (ptr) {
				sscanf(ptr, "\"text\":\"%255[^\"]\"", mensaje);
				printf("Se recibio mensaje: %s\n", mensaje);
			}
			
			// buscar fecha
			ptr = strstr(chunk.response, "\"date\":");
			if (ptr) {
				sscanf(ptr, "\"date\":%[^,]s", fecha);
			}
			
			// logeo mensaje recibido
			if (nuevo_msj) {
				log = fopen("registro_bot.txt", "a");
				if (log != NULL) {
					fprintf(log, "%s | %s | %s\n", fecha, first_name, mensaje);
					fclose(log);
				}
			}
			
			// responder hola
			if (strstr(mensaje, "hola") != NULL || strstr(mensaje, "Hola") != NULL) {
				char reply_url[600];
				sprintf(reply_url,
						"https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=Hola,%%20%s",
						token, chat_id, first_name);
				
				curl_easy_setopt(curl, CURLOPT_URL, reply_url);
				res = curl_easy_perform(curl);
				
				if (res != CURLE_OK)
					printf("Error enviando mensaje: %d\n", res);
				else
					printf("Se respondio: Hola, %s\n", first_name);
				
				log = fopen("registro_bot.txt", "a");
				if (log != NULL) {
					fprintf(log, "%s | Bot | Hola, %s\n", fecha, first_name);
					fclose(log);
				}
			}
			
			// responder chau
			if (strstr(mensaje, "chau") != NULL || strstr(mensaje, "Chau") != NULL) {
				char reply_url[600];
				sprintf(reply_url,
						"https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=Chau,%%20%s",
						token, chat_id, first_name);
				
				curl_easy_setopt(curl, CURLOPT_URL, reply_url);
				res = curl_easy_perform(curl);
				
				if (res != CURLE_OK)
					printf("Error enviando mensaje: %d\n", res);
				else
					printf("Se respondio: Chau, %s\n", first_name);
				
				log = fopen("registro_bot.txt", "a");
				if (log != NULL) {
					fprintf(log, "%s | Bot | Chau, %s\n", fecha, first_name);
					fclose(log);
				}
			}
			
			free(chunk.response);
			printf("-------------------------------------------------\n");
		}
		
		sleep(2);
	}
	
	curl_easy_cleanup(curl);
	return 0;
}
