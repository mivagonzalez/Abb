#ifndef ABB_H
#define ABB_H

#include <stdbool.h>
#include <stddef.h>

typedef struct abb_nodo abb_nodo_t;

typedef struct abb abb_t;

typedef struct abb_iter abb_iter_t;

/* ******************************************************************
 *                    FUNCION EXTRA
 * *****************************************************************/

typedef int (*abb_comparar_clave_t) (const char*, const char*);

typedef void (*abb_destruir_dato_t) (void *);


/* ******************************************************************
 *                    PRIMITIVAS DEL ABB
 * *****************************************************************/

//post: crea un ABB vacio
abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato);

//pre: El ABB fue creado
//post: inserta el dato en el ABB, si tiene memoria, sino devuelve false.
bool abb_guardar(abb_t *arbol, const char *clave, void *dato);

//pre: el arbol esta creado
//post: elimina el elemento del arbol y devuelve el valor del dato asociado
void* abb_borrar(abb_t *arbol, const char *clave);

//pre:el arbol fue creado
//post: devuelve el elemento de la clave correspondiente, si la clave no existe devuelve NULL
void* abb_obtener(const abb_t *arbol, const char *clave);

//pre: el arbol fue creado
//post:devuelve true o false si el elemento existe o no en el arreglo.
bool abb_pertenece(const abb_t *arbol, const char *clave);

//pre: el arbol fue creado.
//post: devuelve la cantidad de elementos que hay en ese arbol.
size_t abb_cantidad(abb_t *arbol);

//pre: el arbol fue creado
//post: destruye el arbol
void abb_destruir(abb_t *arbol);

/* ******************************************************************
 *                  PRIMITIVAS DEL ITERADOR INTERNO
 * *****************************************************************/

//pre: el arbol fue creado
//recorre el arbol de forma in order hasta que visitar devuelva false.
void abb_in_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra);

/* ******************************************************************
 *                  PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/

//pre:el arbol fue creado
//post: crea un iterador del arbol
abb_iter_t* abb_iter_in_crear(const abb_t *arbol);

//pre: el iter fue creado
//post: avanza el iterador de forma inorder
bool abb_iter_in_avanzar(abb_iter_t *iter);

//pre:el iter fue creado
//post: devuelve la clave del nodo que apunta el iter actual;
const char *abb_iter_in_ver_actual(const abb_iter_t *iter);

//pre: el iter fue creado
//post:si ya recorrio todos los nodos imprime true, sino imprime false.
bool abb_iter_in_al_final(const abb_iter_t *iter);

//pre: el arbol fue creado
//post: todos los elementos fueron destuidos.
void abb_iter_in_destruir(abb_iter_t* iter);

#endif // ABB_H
