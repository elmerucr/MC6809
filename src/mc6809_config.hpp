/*
 * mc6809_config.hpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

/*
 * Set to true if the host is little endian.
 *
 * Registers a and b together form the 16 bit d register. Register a is
 * the most significant byte, register b the least significant. The
 * host system deals more efficiently with these registers by referencing
 * a and b to respectively the msb and lsb of the larger d register.
 */
#define HOST_LITTLE_ENDIAN true
