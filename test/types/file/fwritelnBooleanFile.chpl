var myTrueBoolean: bool = true;
var myFalseBoolean = false;
var aSpace = " ";

var f = open("_test_fwritelnBooleanFile.txt", iomode.w).writer();

f.writeln(myTrueBoolean, aSpace, myFalseBoolean);

f.close();

