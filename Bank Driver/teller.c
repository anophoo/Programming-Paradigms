#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "branch.h"
#include "account.h"
#include "error.h"
#include "debug.h"

/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);
  
  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  int branchID = AccountNum_GetBranchID(accountNum);
  sem_wait(&(bank->branches[branchID].branchLock));
  sem_wait(&(account->accLock));  
  Account_Adjust(bank,account, amount, 1);
  sem_post(&(bank->branches[branchID].branchLock));
  sem_post(&(account->accLock));
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  int branchID = AccountNum_GetBranchID(accountNum);
  sem_wait(&(bank->branches[branchID].branchLock));
  sem_wait(&(account->accLock));  

  if (account == NULL) {
    sem_post(&(bank->branches[branchID].branchLock));
    sem_post (&(account->accLock));
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  if (amount > Account_Balance(account)) {
    sem_post(&(bank->branches[branchID].branchLock));
    sem_post (&(account->accLock));
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank,account, -amount, 1);

  sem_post(&(bank->branches[branchID].branchLock));
  sem_post (&(account->accLock));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
//  return ERROR_SUCCESS;
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  if (srcAccountNum == dstAccountNum) {
    return ERROR_SUCCESS;
  }
  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  int srcBranchID, dstBranchID;
  srcBranchID = AccountNum_GetBranchID(srcAccountNum);
  dstBranchID = AccountNum_GetBranchID(dstAccountNum);

  if (updateBranch == 1) {
    if (srcBranchID < dstBranchID) {              //მარტო ბრენჩებით ვამოწმებთ      
        sem_wait(&(bank->branches[srcBranchID].branchLock));   
        sem_wait(&(bank->branches[dstBranchID].branchLock));
        if (srcAccount->accountNumber < dstAccount->accountNumber) {   
           sem_wait(&(srcAccount->accLock));                        
           sem_wait(&(dstAccount->accLock));
        } else {
           sem_wait(&(dstAccount->accLock));                          
           sem_wait(&(srcAccount->accLock));
        }
    } else {
        sem_wait(&(bank->branches[dstBranchID].branchLock));
        sem_wait(&(bank->branches[srcBranchID].branchLock));
        if (srcAccount->accountNumber < dstAccount->accountNumber) {   
           sem_wait(&(srcAccount->accLock));                        
           sem_wait(&(dstAccount->accLock));
        } else {
           sem_wait(&(dstAccount->accLock));                          
           sem_wait(&(srcAccount->accLock));
        }
    }
  } else {
    if (srcAccount->accountNumber < dstAccount->accountNumber) {   
       sem_wait(&(srcAccount->accLock));                        
       sem_wait(&(dstAccount->accLock));
      } else {
       sem_wait(&(dstAccount->accLock));                          
       sem_wait(&(srcAccount->accLock));
      }
    } 
  

  if (amount > Account_Balance(srcAccount)) {
    sem_post(&(dstAccount->accLock));                          
    sem_post(&(srcAccount->accLock));
    if (updateBranch == 1) {
      sem_post(&(bank->branches[srcBranchID].branchLock));   
      sem_post(&(bank->branches[dstBranchID].branchLock));
    }
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);
  if (updateBranch == 1) {
    sem_post(&(bank->branches[srcBranchID].branchLock));   
    sem_post(&(bank->branches[dstBranchID].branchLock));
  }
  sem_post(&(srcAccount->accLock));
  sem_post(&(dstAccount->accLock));                          
  return ERROR_SUCCESS;
}
