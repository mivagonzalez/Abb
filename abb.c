#define _POSIX_C_SOURCE 200809L

#include "abb.h"
#include "pila.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define LADO_IZQUIERDO -1
#define LADO_DERECHO 1



/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/




struct abb_nodo {
	char* clave;
	void* dato;
	abb_nodo_t* izq;
	abb_nodo_t* der;
};

struct abb {
	abb_nodo_t* raiz;
	abb_comparar_clave_t cmp;
	abb_destruir_dato_t destruir_dato;
	size_t cantidad;
};

struct abb_iter {
	const abb_t* arbol;
    pila_t* pila;
};

/* ******************************************************************
 *                    FUNCIONES AUXILIARES
 * *****************************************************************/

abb_nodo_t* abb_nodo_crear(const char* clave, void* dato) {
	abb_nodo_t* nodo = malloc(sizeof(abb_nodo_t));
	if(!nodo) return NULL;
	nodo->clave = strndup(clave, strlen(clave));
	nodo->dato = dato;
	nodo->izq = NULL;
	nodo->der = NULL;
	return nodo;
}


abb_nodo_t* abb_nodo_buscar(abb_nodo_t* nodo, const char* clave, abb_comparar_clave_t cmp) { 
	if (!nodo) return NULL;
	if (cmp(clave, nodo->clave) == 0) return nodo;
	if (cmp(clave, nodo->clave) < 0) return abb_nodo_buscar (nodo->izq,clave,cmp);
	return abb_nodo_buscar (nodo->der,clave,cmp);
}


bool abb_nodo_insertar(abb_t* arbol, abb_nodo_t* nodo, const char* clave, void* dato) { 
	int cmp = arbol->cmp(nodo->clave, clave);

	if(cmp == 0) {
		if(arbol->destruir_dato) arbol->destruir_dato(nodo->dato);
		nodo->dato = dato;
		return true;
	} 
	if(cmp > 0) {
		if(nodo->izq) return abb_nodo_insertar(arbol, nodo->izq, clave, dato);
		nodo->izq = abb_nodo_crear(clave, dato);
		if(!nodo->izq) return false;
	} else {
		if(nodo->der) return abb_nodo_insertar(arbol, nodo->der, clave, dato);
		nodo->der = abb_nodo_crear(clave, dato);
		if(!nodo->der) return false;
	}	
	arbol->cantidad++;
	return true;
}

abb_nodo_t* buscar_ultimo_padre_izq(abb_nodo_t* nodo) {
	if(!nodo || !nodo->izq) return NULL;
	if(!nodo->izq->izq) return nodo;
	return buscar_ultimo_padre_izq(nodo->izq);
}

void asignar_hijos_a_reemplazante_zurdo(abb_nodo_t* padre_del_reempl, abb_nodo_t* nodo) {
	abb_nodo_t* reemplazante = padre_del_reempl->izq;
	padre_del_reempl->izq = reemplazante->der;
	reemplazante->izq = nodo->izq;
	reemplazante->der = nodo->der;
	return;
}

void* abb_nodo_destruir(abb_nodo_t* nodo) {
	if (!nodo) return NULL;
	void* dato = nodo->dato;
	free(nodo->clave);
	free(nodo);
	return dato;
}

void* abb_borrar_hoja(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, abb_t* arbol) {
	if (!padre) arbol->raiz = NULL;
	else if (lado == LADO_DERECHO) padre->izq = NULL;
	else if (lado == LADO_IZQUIERDO) padre->der = NULL;
	arbol->cantidad--;
	return abb_nodo_destruir(nodo);
}

void* abb_borrar_con_hijo_unico(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, abb_t* arbol) {
	abb_nodo_t* reemplazante;
	if (!nodo->der)	reemplazante = nodo ->izq;
	else reemplazante = nodo->der;
	if (!padre) arbol->raiz = reemplazante;
	else if (lado == LADO_DERECHO) padre->izq = reemplazante;
	else padre->der = reemplazante;
	arbol->cantidad--;
	return abb_nodo_destruir(nodo);
}

char* buscar_reemplazante (abb_nodo_t* nodo) {
	if (!nodo) return NULL;
	if (! nodo->izq) return nodo->clave;
	return buscar_reemplazante(nodo->izq);
}

void* abb_borrar_2hijos(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, abb_t* arbol) {
	char* clave_reemplazante = strdup(buscar_reemplazante(nodo->der));
	void* dato_reemplazante = abb_borrar(arbol,clave_reemplazante);
	free (nodo->clave);
	void* dato_atual = nodo->dato;
	nodo->clave = clave_reemplazante;
	nodo->dato = dato_reemplazante;
	return dato_atual;
}

bool tiene_un_solo_hijo(abb_nodo_t* nodo) {
	return (!nodo->izq  && nodo->der) || (!nodo->der && nodo->izq);
}

bool no_tiene_hijos(abb_nodo_t* nodo) {
	return !nodo->izq && !nodo->der;	
}

void* _abb_borrar(abb_nodo_t* nodo, abb_nodo_t* padre, int lado, const char* clave, abb_t* arbol) {
	int cmp = arbol->cmp(nodo->clave, clave);
	if(cmp == 0) {
		if (no_tiene_hijos(nodo)) {
			return abb_borrar_hoja(nodo, padre, lado, arbol);
		} else if (tiene_un_solo_hijo(nodo)) {
			return abb_borrar_con_hijo_unico(nodo, padre, lado, arbol);
		}
		return abb_borrar_2hijos(nodo, padre, lado, arbol);
	}
	if (cmp > 0) {
		return _abb_borrar(nodo->izq, nodo, LADO_DERECHO , clave, arbol);
	}
	if (cmp < 0) {
		return _abb_borrar(nodo->der, nodo, LADO_IZQUIERDO, clave, arbol);
	}
	return NULL;
}


bool _apilar(pila_t* pila, abb_nodo_t* raiz) {
	if (!raiz) return false;
	pila_apilar(pila, raiz);
	if (raiz->izq) {
		_apilar(pila, raiz->izq);	
	} 
	return true;
}

void _abb_destruir(abb_nodo_t* nodo, abb_destruir_dato_t destruir_dato) {
	if (!nodo) return;
	_abb_destruir(nodo->izq, destruir_dato);
	_abb_destruir(nodo->der, destruir_dato);	
	void* dato = abb_nodo_destruir(nodo);
	if (destruir_dato) {
		destruir_dato(dato);
	}
	return;
}

/* ******************************************************************
 *                    ITERADOR INTERNO
 * *****************************************************************/

void iter_in_order(abb_nodo_t* nodo, bool visitar (const char*, void*, void*), void* extra, bool* continuar){
	if (!nodo) return;
	iter_in_order(nodo->izq,visitar,extra,continuar);
	if (! (*continuar)) return;
	(*continuar) = visitar(nodo->clave,nodo->dato,extra);
	iter_in_order(nodo->der,visitar,extra,continuar);
}

void abb_in_order(abb_t *arbol, bool visitar(const char*, void*, void*), void* extra) {
	if (!visitar || !arbol->raiz) return;
	bool continuar = true;
	iter_in_order(arbol->raiz,visitar,extra,&continuar);
}

/* ******************************************************************
 *                    PRIMITIVAS DEL ABB
 * *****************************************************************/

abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato) {
	abb_t* abb = malloc(sizeof(abb_t));
	if (!abb) return NULL;
	abb->raiz = NULL;
	if(cmp) abb->cmp = cmp;
	abb->destruir_dato = destruir_dato;
	abb->cantidad = 0;
	return abb;
}

bool abb_guardar(abb_t* arbol, const char* clave, void* dato) {
	if(!arbol->raiz) {
		arbol->raiz = abb_nodo_crear(clave, dato);
		if(!arbol->raiz) return false;
		arbol->cantidad++;
		return true;
	} 
	bool insertado = abb_nodo_insertar(arbol, arbol->raiz, clave, dato);
	return insertado;
}

void* abb_obtener(const abb_t *arbol, const char *clave) {
	if(!arbol->raiz) return NULL;
	abb_nodo_t* nodo = abb_nodo_buscar(arbol->raiz, clave, arbol->cmp);
	if(!nodo) return NULL;
	return nodo->dato;
}

size_t abb_cantidad(abb_t* arbol) {
	return arbol->cantidad;
}

bool abb_pertenece(const abb_t *arbol, const char *clave) {
	if(!arbol->raiz) return false;
	return abb_nodo_buscar(arbol->raiz, clave, arbol->cmp) != NULL;
}

void* abb_borrar(abb_t *arbol, const char *clave) {
	if (!arbol) return NULL;
	if (!abb_pertenece(arbol, clave)) return NULL;
	return _abb_borrar(arbol->raiz, NULL, 0, clave, arbol);	
}

void abb_destruir(abb_t* arbol) {
	if (!arbol) return;
	if (arbol->raiz) {
		_abb_destruir(arbol->raiz, arbol->destruir_dato);
	}
	free (arbol);
}

/* ******************************************************************
 *                    ITERADOR EXTERNO
 * *****************************************************************/

abb_iter_t* abb_iter_in_crear(const abb_t *arbol) {
	abb_iter_t* iter = malloc(sizeof(abb_iter_t));
	if(!iter) return NULL;
	pila_t* pila = pila_crear();
	if(!pila) {
		free(iter);
		return NULL;
	}
	iter->pila = pila;	
	iter->arbol = arbol;
	_apilar(iter->pila, arbol->raiz);
	return iter;
}

bool abb_iter_in_avanzar(abb_iter_t *iter) {
	if(abb_iter_in_al_final(iter)) return false;
	abb_nodo_t* nodo = pila_desapilar(iter->pila);
	if(nodo->der) {
		if(!_apilar(iter->pila, nodo->der)) return false;
	}
	return true;
}

const char* abb_iter_in_ver_actual(const abb_iter_t *iter) {
	if(abb_iter_in_al_final(iter)) return NULL;
	abb_nodo_t* nodo = pila_ver_tope(iter->pila);
	return nodo->clave;
}

bool abb_iter_in_al_final(const abb_iter_t *iter) {
	return pila_esta_vacia(iter->pila);
}

void abb_iter_in_destruir(abb_iter_t* iter) {
	pila_destruir(iter->pila);
	free(iter);
}