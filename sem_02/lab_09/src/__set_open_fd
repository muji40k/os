static inline void __set_open_fd(unsigned int fd, struct fdtable *fdt)
{
	__set_bit(fd, fdt->open_fds);
	fd /= BITS_PER_LONG;
	if (!~fdt->open_fds[fd])
		__set_bit(fd, fdt->full_fds_bits);
}
