#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char id[100];
  char title[100];
  List *genres;
  char director[300];
  float rating;
  int year;
} Film;

void mostrarMenuPrincipal() {
  limpiarPantalla();

  puts("========================================");
  puts("     Base de Datos de Películas");
  puts("========================================");

  puts("1) Cargar Películas");
  puts("2) Buscar por id");
  puts("3) Buscar por director");
  puts("4) Buscar por género");
  puts("5) Buscar por década");
  puts("6) Buscar por rango de calificaciones");
  puts("7) Buscar por década y género");
  puts("8) Salir");
}

int is_equal_str(void *key1, void *key2) {
  return strcmp((char *)key1, (char *)key2) == 0;
}

int is_equal_int(void *key1, void *key2) {
  return *(int *)key1 == *(int *)key2;
}

void mostrar_pelicula(Film *peli) {
  printf("ID: %s\n", peli->id);
  printf("Título: %s\n", peli->title);
  printf("Director: %s\n", peli->director);
  printf("Año: %d\n", peli->year);
  printf("Calificación: %.1f\n", peli->rating);

  printf("Géneros: ");

  char *genero = list_first(peli->genres);

  while (genero != NULL) {
    printf("%s", genero);
    genero = list_next(peli->genres);

    if (genero != NULL) {
      printf(", ");
    }
  }

  printf("\n\n");
}

void cargar_peliculas(Map *pelis_byid, Map *pelis_bygenres,
                      Map *pelis_bydirector, Map *pelis_bydecade) {

  // Evita duplicar las películas si se elige la opción 1 más de una vez
  if (map_first(pelis_byid) != NULL) {
    printf("Las películas ya fueron cargadas.\n");
    return;
  }

  FILE *archivo = fopen("data/Top1500.csv", "r");

  if (archivo == NULL) {
    perror("Error al abrir el archivo");
    return;
  }

  char **campos = leer_linea_csv(archivo, ',');

  while ((campos = leer_linea_csv(archivo, ',')) != NULL) {

    Film *peli = (Film *)malloc(sizeof(Film));

    if (peli == NULL) {
      fclose(archivo);
      return;
    }

    strcpy(peli->id, campos[1]);
    strcpy(peli->title, campos[5]);
    strcpy(peli->director, campos[14]);

    peli->genres = split_string(campos[11], ",");
    peli->year = atoi(campos[10]);
    peli->rating = atof(campos[8]); // campo "IMDb Rating" de Top1500.csv

    map_insert(pelis_byid, peli->id, peli);

    // Mapa por género: cada género apunta a una lista de películas
    char *genero = list_first(peli->genres);

    while (genero != NULL) {

      MapPair *par = map_search(pelis_bygenres, genero);

      if (par == NULL) {
        List *lista = list_create();

        if (lista != NULL) {
          list_pushBack(lista, peli);
          map_insert(pelis_bygenres, genero, lista);
        }
      } else {
        List *lista = par->value;
        list_pushBack(lista, peli);
      }

      genero = list_next(peli->genres);
    }

    // Mapa por director: una película puede tener varios directores
    // (ej. "Joel Coen, Ethan Coen"), así que se registra con cada uno
    List *directores = split_string(campos[14], ",");
    char *director = list_first(directores);

    while (director != NULL) {

      MapPair *parDirector = map_search(pelis_bydirector, director);

      if (parDirector == NULL) {

        List *lista = list_create();

        if (lista != NULL) {
          list_pushBack(lista, peli);
          map_insert(pelis_bydirector, director, lista);
        }

      } else {

        List *lista = parDirector->value;
        list_pushBack(lista, peli);
      }

      director = list_next(directores);
    }

    // Mapa por década
    int decada = (peli->year / 10) * 10;
    char claveDecada[20];

    sprintf(claveDecada, "%d", decada);

    MapPair *parDecada = map_search(pelis_bydecade, claveDecada);

    if (parDecada == NULL) {

      List *lista = list_create();

      if (lista != NULL) {

        list_pushBack(lista, peli);

        char *clave = malloc(strlen(claveDecada) + 1);

        if (clave != NULL) {
          strcpy(clave, claveDecada);
          map_insert(pelis_bydecade, clave, lista);
        }
      }

    } else {

      List *lista = parDecada->value;
      list_pushBack(lista, peli);
    }
  }

  fclose(archivo);
}

void buscar_por_id(Map *pelis_byid) {

  char id[100];

  printf("Ingrese el id de la película: ");
  scanf("%99s", id);

  MapPair *par = map_search(pelis_byid, id);

  if (par != NULL) {

    Film *peli = par->value;
    mostrar_pelicula(peli);

  } else {

    printf("La película con id %s no existe\n", id);
  }
}

void buscar_por_director(Map *pelis_bydirector) {

  char director[300];

  printf("Ingrese el director: ");
  scanf(" %299[^\n]", director);

  MapPair *par = map_search(pelis_bydirector, director);

  if (par == NULL) {
    printf("No se encontraron películas de ese director.\n");
    return;
  }

  List *peliculas = par->value;
  Film *peli = list_first(peliculas);

  while (peli != NULL) {
    mostrar_pelicula(peli);
    peli = list_next(peliculas);
  }
}

void buscar_por_genero(Map *pelis_bygenres) {

  char genero[100];

  printf("Ingrese el género: ");
  scanf("%99s", genero);

  MapPair *par = map_search(pelis_bygenres, genero);

  if (par == NULL) {
    printf("No se encontraron películas de ese género.\n");
    return;
  }

  List *peliculas = par->value;
  Film *peli = list_first(peliculas);

  while (peli != NULL) {
    mostrar_pelicula(peli);
    peli = list_next(peliculas);
  }
}

void buscar_por_decada(Map *pelis_bydecade) {

  int decada;

  printf("Ingrese la década: ");
  scanf("%d", &decada);

  decada = (decada / 10) * 10;

  char clave[20];
  sprintf(clave, "%d", decada);

  MapPair *par = map_search(pelis_bydecade, clave);

  if (par == NULL) {
    printf("No se encontraron películas de esa década.\n");
    return;
  }

  List *peliculas = par->value;
  Film *peli = list_first(peliculas);

  while (peli != NULL) {
    mostrar_pelicula(peli);
    peli = list_next(peliculas);
  }
}

void buscar_por_rating(Map *pelis_byid) {

  float minimo;
  float maximo;

  printf("Ingrese la calificación mínima: ");
  scanf("%f", &minimo);

  printf("Ingrese la calificación máxima: ");
  scanf("%f", &maximo);

  MapPair *par = map_first(pelis_byid);

  while (par != NULL) {

    Film *peli = par->value;

    if (peli->rating >= minimo && peli->rating <= maximo) {
      mostrar_pelicula(peli);
    }

    par = map_next(pelis_byid);
  }
}

int pelicula_tiene_genero(Film *peli, char *generoBuscado) {

  char *genero = list_first(peli->genres);

  while (genero != NULL) {

    if (strcmp(genero, generoBuscado) == 0) {
      return 1;
    }

    genero = list_next(peli->genres);
  }

  return 0;
}

void buscar_por_decada_genero(Map *pelis_bydecade) {

  int decada;
  char genero[100];

  printf("Ingrese la década: ");
  scanf("%d", &decada);

  printf("Ingrese el género: ");
  scanf("%99s", genero);

  decada = (decada / 10) * 10;

  char clave[20];
  sprintf(clave, "%d", decada);

  MapPair *par = map_search(pelis_bydecade, clave);

  if (par == NULL) {
    printf("No se encontraron películas de esa década.\n");
    return;
  }

  List *peliculas = par->value;
  Film *peli = list_first(peliculas);

  while (peli != NULL) {

    if (pelicula_tiene_genero(peli, genero)) {
      mostrar_pelicula(peli);
    }

    peli = list_next(peliculas);
  }
}

int main() {

  char opcion;

  // Un mapa por criterio de búsqueda (TDA Mapa de tdas/map.h)
  Map *pelis_byid = map_create(is_equal_str);
  Map *pelis_bygenres = map_create(is_equal_str);
  Map *pelis_bydirector = map_create(is_equal_str);
  Map *pelis_bydecade = map_create(is_equal_str);

  do {

    mostrarMenuPrincipal();

    printf("Ingrese su opción: ");
    scanf(" %c", &opcion);

    switch (opcion) {

    case '1':
      cargar_peliculas(
          pelis_byid,
          pelis_bygenres,
          pelis_bydirector,
          pelis_bydecade);
      break;

    case '2':
      buscar_por_id(pelis_byid);
      break;

    case '3':
      buscar_por_director(pelis_bydirector);
      break;

    case '4':
      buscar_por_genero(pelis_bygenres);
      break;

    case '5':
      buscar_por_decada(pelis_bydecade);
      break;

    case '6':
      buscar_por_rating(pelis_byid);
      break;

    case '7':
      buscar_por_decada_genero(pelis_bydecade);
      break;

    case '8':
      break;

    default:
      printf("Opción inválida.\n");
      break;
    }

    if (opcion != '8') {
      presioneTeclaParaContinuar();
    }

  } while (opcion != '8');

  return 0;
}
