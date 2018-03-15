#ifndef _BANK_H
#define _BANK_H
#include <semaphore.h>

typedef struct Bank {
  unsigned int numberBranches; // რამდენი ბრენჩი მუშაობს
  struct       Branch  *branches;
  struct       Report  *report;
  int numberWorkersHasToFinish; // სახელი შესაცვლელი
  /* ამოწმებს, ყველა ბრენჩმა დაამთავრა თუ არა მუშაობა 
  და numberWorkersHasToFinish-ს ამცირებს */
  sem_t forCheck; // სახელი შესაცვლელი

  /* ამოწმებს, არის თუ არა ბრენჩი მზად, დაიწყოს მეორე დღე */
  sem_t letMeStartNextDay;	// სახელი შესაცვლელი

  sem_t lockForReportTransfer;
} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);
void Bank_Dispose(Bank *bank);
Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);


#endif /* _BANK_H */
