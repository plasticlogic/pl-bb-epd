/*
 * parallel.h
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#ifndef PARALLEL_H_
#define PARALLEL_H_

typedef struct pl_parallel
{
  void *hw_ref;		// hardware reference
  int fd;           // open file descriptor: /dev/spi-X.Y
  int cs_gpio; 		// chip select gpio

  int (*open)(struct pl_parallel * p);
  int (*close)(struct pl_parallel * p);
  int (*read_bytes)(struct pl_parallel *p, uint8_t *buff, size_t size);
  int (*write_bytes)(struct pl_parallel *p, uint8_t *buff, size_t size);
  int (*set_cs)(struct pl_parallel *p, uint8_t cs);
  void (*delete)(struct pl_parallel *p);

  void *mSPI;
} pl_parallel_t;

#endif /* PARALLEL_H_ */
