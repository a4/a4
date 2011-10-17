case FieldDescriptor::CPPTYPE_BOOL:
    TRY_MATCH(Bool_t,   bool,           bool);
    TRY_MATCH(Int_t,    int,            bool);  
    TRY_MATCH(Long_t,   long,           bool);
    TRY_MATCH(Short_t,  short,          bool);
    TRY_MATCH(Char_t,   char,           bool);
    TRY_MATCH(UInt_t,   unsigned int,   bool);
    TRY_MATCH(ULong_t,  unsigned long,  bool);
    TRY_MATCH(UShort_t, unsigned short, bool);
    TRY_MATCH(UChar_t,  unsigned char,  bool);
    FAILURE("FieldDescriptor::CPPTYPE_BOOL");

case FieldDescriptor::CPPTYPE_INT32:
    TRY_MATCH(Int_t,   int,   int32_t);
    TRY_MATCH(Long_t,  long,  int32_t);
    TRY_MATCH(Short_t, short, int32_t);
    TRY_MATCH(Char_t,  char,  int32_t);
    FAILURE("FieldDescriptor::CPPTYPE_INT32");

case FieldDescriptor::CPPTYPE_UINT32:
    TRY_MATCH(UInt_t,   unsigned int,   uint32_t);
    TRY_MATCH(ULong_t,  unsigned long,  uint32_t);
    TRY_MATCH(UShort_t, unsigned short, uint32_t);
    TRY_MATCH(UChar_t,  unsigned char,  uint32_t);
    FAILURE("FieldDescriptor::CPPTYPE_UINT32");

case FieldDescriptor::CPPTYPE_INT64:
    TRY_MATCH(Long64_t, long long, int64_t);
    TRY_MATCH(Int_t,    int,       int64_t);
    TRY_MATCH(Long_t,   long,      int64_t);
    TRY_MATCH(Short_t,  short,     int64_t);
    TRY_MATCH(Char_t,   char,      int64_t);
    FAILURE("FieldDescriptor::CPPTYPE_INT64");
    
case FieldDescriptor::CPPTYPE_UINT64:
    TRY_MATCH(ULong64_t, unsigned long long, uint64_t);
    TRY_MATCH(UInt_t,    unsigned int,       uint64_t);
    TRY_MATCH(ULong_t,   unsigned long,      uint64_t);
    TRY_MATCH(UShort_t,  unsigned short,     uint64_t);
    TRY_MATCH(UChar_t,   unsigned char,      uint64_t);
    FAILURE("FieldDescriptor::CPPTYPE_UINT64");

case FieldDescriptor::CPPTYPE_FLOAT:
    TRY_MATCH(Float_t, float, float);
    FAILURE("FieldDescriptor::CPPTYPE_FLOAT");
    
case FieldDescriptor::CPPTYPE_DOUBLE:
    TRY_MATCH(Double_t, double, double);
    TRY_MATCH(Float_t,  float,  double);
    FAILURE("FieldDescriptor::CPPTYPE_DOUBLE");

case FieldDescriptor::CPPTYPE_STRING:
{
    using std::string;
    TRY_MATCH(string, string, std::string);
    FAILURE("FieldDescriptor::CPPTYPE_STRING");
}
