/* 
 * File:   ValidateCheck.h
 * Author: Ryan Kophs
 *
 * Created on November 14, 2013, 1:43 PM
 */
#pragma once

#include "include/global.h"
#include <string>
#include "Range.h"

struct ValidateChecks {
   ValidateChecks();
   virtual ~ValidateChecks();
   LR_VIRTUAL bool CheckNumber(const std::string& number, const Range& range);
   LR_VIRTUAL void CheckNumberForNegative(const std::string& number); // throw
   LR_VIRTUAL void CheckNumberForSize(const std::string& number, const Range& range); // throw

   LR_VIRTUAL bool CheckString(const std::string& text);
   LR_VIRTUAL void CheckStringForSize(const std::string& text); // throw

   LR_VIRTUAL bool CheckBool(const std::string& text);
};

