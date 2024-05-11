// SPDX-License-Identifier: GPL-2.0
#include <string.h>
#include <stdio.h>

//
//rate of progress output
//
void ProgressInit(void)
{
  char prog[] = "[                                                                        ]   0%";

  printf("%s", prog);
}

void ProgressShow(int percentage)
{
  char prog[] = "[                                                                        ]   0%";
  char per[5];
  int count;
  int i;

  if (percentage > 100) percentage = 100;
  if (percentage < 0) percentage = 0;

  count = percentage * 72 / 100 - 1;

  for (i = 1; i <= count; i++)
    prog[i] = '=';
  prog[i] = '>';

  sprintf(per, "%u%%", percentage);

  memcpy(prog + strlen(prog) - strlen(per), per, strlen(per));

  printf("\r%s", prog);
}

void ProgressDone(void)
{
  char prog[] = "[========================================================================] 100%";

  printf("\r%s\n", prog);
}

