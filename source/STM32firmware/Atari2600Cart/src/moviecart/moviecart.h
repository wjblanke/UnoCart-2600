#ifndef MOVIECART_H
#define MOVIECART_H

#include <stdbool.h>

/*
 * Play the MovieCart file at path ("/DIR/FILE.MVC", 8.3 names). Prepares the
 * MovieCart kernel, calls start_cart() to serve the menu until it reboots
 * into the cartridge, then serves MovieCart until power-off. Never returns.
 */
void moviecart_launch(const char *path, bool (*start_cart)(void))
	__attribute__((noreturn));

#endif /* MOVIECART_H */
