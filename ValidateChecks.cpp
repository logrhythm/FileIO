//#include "ValidateChecks.h"
//#include <sstream>
//#include <stdexcept>
//
//namespace {
//   // is only used internally in this class but is less than nice 
//   // to keep here. 
//   // Kjell's suggestion: 
//   // I would keep it here and move it later when/if the validation
//   // is extracted to its own class,. which should? be done when 
//   // the other conf are being validated
//
//   class ConfValidationException : public std::runtime_error {
//   public:
//
//      /// constructs an internal exception
//
//      explicit ConfValidationException(const std::string& msg)
//      : std::runtime_error(msg) {
//      }
//   };
//}
//
//ValidateChecks::ValidateChecks() {
//
//}
//
//ValidateChecks::~ValidateChecks() {
//
//}
//
///**
// * Check if string can be converted to a number and is within allowed limits
// * max value and being positive. 
// * 
// * @param number to be check for number and number size
// * @param max value of the number
// * @throws on illegal values
// */
//bool ValidateChecks::CheckNumber(const std::string& number, const std::shared_ptr<RangeInt> range) {
//   try {
//      CheckNumberForNegative(number);
//      CheckNumberForSize(number, range);
//   } catch (std::exception e) {
//      LOG(WARNING) << "Number not accepted: " << e.what();
//      return false;
//   }
//
//   return true;
//}
//
///**
// * Check if string can be converted to a number and is not negative
// * @param number to be check for number and being positive
// * @throws on illegal values
// */
//void ValidateChecks::CheckNumberForNegative(const std::string& number) {
//   int32_t value = std::stoi(number);
//   if (value < 0) {
//      std::string error = {"Negative values not accepted in BaseConf : "};
//      error.append(std::to_string(value));
//      throw ConfValidationException(error);
//   }
//}
//
///**
// * Check if string can be converted to a number and is within allowed size
// * @param number to be check for number and number size
// * @param max value of the number
// * @throws on illegal values
// */
//void ValidateChecks::CheckNumberForSize(const std::string& number, const std::shared_ptr<RangeInt> range) {
//   if (!(*range).Validate(number)) {
//      std::string error = {"Illegal value. Must be within range of: "};
//      error.append(std::to_string((*range).lower));
//      error.append("  - ").append(std::to_string((*range).higher));
//      error.append(". This value was: ").append(number);
//      throw ConfValidationException(error);
//   }
//}
//
///**
// * Check is string is valid to be used internally
// * @param text to check checked
// * @return true if legal to use
// */
//bool ValidateChecks::CheckString(const std::string& text, const std::shared_ptr<RangeString> range) {
//   try {
//      CheckStringForSize(text, range);
//   } catch (std::exception e) {
//      LOG(WARNING) << "Text/String value not accepted: " << e.what();
//      return false;
//   }
//
//   return true;
//}
//
///**
// * Check if string is within reasonable size limits. Too large strings are illegal.
// * @param string to be check for size
// * @throws on illegal size
// */
//void ValidateChecks::CheckStringForSize(const std::string& text, const std::shared_ptr<RangeString> range) {
//   if (!(*range).Validate(text)) {
//      std::string error = {"Illegal string size, Must be within range of: "};
//      error.append(std::to_string((*range).lower));
//      error.append(" - ").append(std::to_string((*range).higher));
//      error.append(". This size was: ").append(std::to_string(text.size()));
//      throw ConfValidationException(error);
//   }
//}
//
///**
// * Check if string can be converted to bool
// * @param string to be check for "false" or "true". "1" and "0" are illegal values
// * @return true if bool can be extracted from string
// */
//bool ValidateChecks::CheckBool(const std::string& text, const std::shared_ptr<RangeBool> range) {
//   if ((*range).Validate(text)) {
//      return true;
//   }
//   LOG(WARNING) << "Illegal bool, received value of size: " << text.size() << " Expected: " << (*range).True << " or " << (*range).False << ".";
//   return false;
//}
