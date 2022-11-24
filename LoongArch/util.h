/*
 * Copyright (c) 2022 yetist <yetist@localhost>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * */

#ifndef __UTIL_H__
#define __UTIL_H__ 1

void *vtpa(unsigned long long vaddr,int fd);
int releaseMem(void *p);
void hexdump (unsigned long bse, char *buf, int len);
void* parse_mac(char *szMacStr);

#endif /* end of include __UTIL_H__ */
