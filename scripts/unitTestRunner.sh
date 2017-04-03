# The FileIO function "ReadAsciiFileContentAsRoot" modifies the permissions of the
#   process running it. For this reason, code coverage generation fails. It was 
#   previously thought that running tests as root was the culprit; that is not true.
#   Running tests as root is fine, but this particular test needs to have its .gcda
#   file generated first, then filled back in appropriately.
#
set -x
sudo ./UnitTestRunner --gtest_filter=-*TestReadAsciiFileContentAsRoot
sudo chown -R $USER:$USER .
