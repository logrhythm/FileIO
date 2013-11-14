/* 
 * File:   ValidateCheck.h
 * Author: Ryan Kophs
 *
 * Created on November 14, 2013, 1:43 PM
 */
#pragma once

#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include "BaseConfMsg.pb.h"
#include "SyslogConfMsg.pb.h"
#include "QosmosConf.h"
#include "include/global.h"
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

