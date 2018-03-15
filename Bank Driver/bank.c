#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>


#include "error.h"

#include "bank.h"
#include "branch.h"
#include "account.h"
#include "report.h"


/*
 * allocate the bank structure and initialize the branches.
 */
Bank*
Bank_Init(int numBranches, int numAccounts, AccountAmount initalAmount,
          AccountAmount reportingAmount,
          int numWorkers)
{
  Bank *bank = malloc(sizeof(Bank));

  if (bank == NULL) {
    return bank;
  }

  bank-> numberWorkersHasToFinish = numWorkers;  // რამდენი ბრენჩი მუშაობს
  sem_init(&(bank->forCheck), 0, 1); // 1 იმიტომ, რომ ნებისმიერი იწყებს
  sem_init(&(bank->letMeStartNextDay), 0, 0); // არავის უნდა ჯერ მეორე დღეზე გადასვლა
  sem_init(&(bank->lockForReportTransfer), 0, 1); // Report_Transfer-ის ატომურობიზა
  
  Branch_Init(bank, numBranches, numAccounts, initalAmount);
  Report_Init(bank, reportingAmount, numWorkers);

  return bank;
}

void Bank_Dispose(Bank *bank) {
  if (bank == NULL) 
    return;
  sem_destroy(&(bank->forCheck));
  sem_destroy(&(bank->letMeStartNextDay));
  sem_destroy(&(bank->lockForReportTransfer));
  for (int i = 0; i < bank->numberBranches; i++)
  {
    Branch *branch = &bank->branches[i];
    sem_destroy(&(branch->branchLock));
    for (int i = 0; i < branch->numberAccounts; ++i)
    {
      sem_destroy(&branch->accounts[i].accLock);
    }
    free(branch->accounts);
  }
  free(bank->report);
  free(bank->branches);
  free(bank);
}

/*
 * get the balance of the entire bank by adding up all the balances in
 * each branch.
 */
int
Bank_Balance(Bank *bank, AccountAmount *balance)
{
  assert(bank->branches);
  for (int i = 0; i < bank->numberBranches; ++i)
  {
    sem_wait(&bank->branches[i].branchLock);
  }

  AccountAmount bankTotal = 0;
  for (unsigned int branch = 0; branch < bank->numberBranches; branch++) {
    AccountAmount branchBalance;
    int err = Branch_Balance(bank,bank->branches[branch].branchID, &branchBalance);
    if (err < 0) {
      for (int i = 0; i < bank->numberBranches; ++i)
      {
        sem_post(&bank->branches[i].branchLock);
      }
      return err;
    }
    bankTotal += branchBalance;
  }

  *balance = bankTotal;

  for (int i = 0; i < bank->numberBranches; ++i)
  {
    sem_post(&bank->branches[i].branchLock);
  }
  return 0;
}

/*
 * tranverse and validate each branch.
 */
int
Bank_Validate(Bank *bank)
{
  assert(bank->branches);
  int err = 0;

  for (unsigned int branch = 0; branch < bank->numberBranches; branch++) {
    int berr = Branch_Validate(bank,bank->branches[branch].branchID);
    if (berr < 0) {
      err = berr;
    }
  }
  return err;
}

/*
 * compare the data inside two banks and see they are exactly the same;
 * it is called in BankTest.
 */
int
Bank_Compare(Bank *bank1, Bank *bank2)
{
  int err = 0;
  if (bank1->numberBranches != bank2->numberBranches) {
    fprintf(stderr, "Bank num branches mismatch\n");
    return -1;
  }

  for (unsigned int branch = 0; branch < bank1->numberBranches; branch++) {
    int berr = Branch_Compare(&bank1->branches[branch],
                              &bank2->branches[branch]);
    if (berr < 0) {
      err = berr;
    }
  }

  int cerr = Report_Compare(bank1, bank2);
  if (cerr < 0)
    err = cerr;

  return err;

}
