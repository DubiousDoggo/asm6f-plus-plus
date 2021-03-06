#ifndef ASM6FPP_HPP
#define ASM6FPP_HPP

#include <cstdint>
#include <string>
#include <map>

#define VERSION "1.7"

#define addr firstlabel.value // '$' value
#define NOORIGIN -0x40000000  // nice even number so aligning works before origin is defined
#define INITLISTSIZE 128      // initial label list size
#define BUFFSIZE 8192         // file buffer (inputbuff, outputbuff) size
#define WORDMAX 128           // used with getword()
#define LINEMAX 2048          // plenty of room for nested equates
#define MAXPASSES 7           // number of tries before giving up
#define IFNESTS 32            // max nested .IF directives
#define DEFAULTFILLER 0       // default fill value
#define LOCALCHAR '@'         // character to define local labels
#define LISTMAX 8             // number of output bytes to show in listing

typedef uint8_t byte;
const byte END_BYTE = -1;

enum labeltype
{
  LABEL,    // known address
  VALUE,    // defined with '='
  EQUATE,   // defined with EQU
  MACRO,    // macro definition
  RESERVED, // reserved word (opcode, directive, etc.)
};

enum cdltype
{
  NONE = 0,
  CODE = 1,
  DATA = 2
};

// operand type, also called addressing mode
// The ordering here is important for parsing
enum optype
{
  ACC, // Accumulator
  IMM, // Immediate
  IND, // Indirect
  IDX, // X-Indexed, Indirect
  IDY, // Indirect, Y-Inexed
  ZPX, // Zero Page, X-Indexed
  ZPY, // Zero Page, Y-Indexed
  ABX, // Absolute, X-Indexed
  ABY, // Absolute, Y-Indexed
  ZPG, // Zero Page
  ABS, // Absolute
  REL, // Relative
  IMP, // Implied
};

const char *ophead[] = {
    "",  // Accumulator
    "#", // Immediate
    "(", // Indirect
    "(", // X-Indexed, Indirect
    "(", // Indirect, Y-Indexed
    "",  // Zero Page, X-Indexed
    "",  // Zero Page, Y-Indexed
    "",  // Absolute, X-Indexed
    "",  // Absolute, Y-Indexed
    "",  // Zero Page
    "",  // Absolute
    "",  // Relative
    "",  // Implied
};

const char *optail[] = {
    "A",   // Accumulator
    "",    // Immediate
    ")",   // Indirect
    ",X)", // X-Indexed, Indirect
    "),Y", // Indirect, Y-Inexed
    ",X",  // Zero Page, X-Indexed
    ",Y",  // Zero Page, Y-Indexed
    ",X",  // Absolute, X-Indexed
    ",Y",  // Absolute, Y-Indexed
    "",    // Zero Page
    "",    // Absolute
    "",    // Relative
    "",    // Implied
};

// number of bytes an operand type takes
const unsigned opsize[] = {
    0, // Accumulator
    1, // Immediate
    2, // Indirect
    1, // X-Indexed, Indirect
    1, // Indirect, Y-Inexed
    1, // Zero Page, X-Indexed
    1, // Zero Page, Y-Indexed
    2, // Absolute, X-Indexed
    2, // Absolute, Y-Indexed
    1, // Zero Page
    2, // Absolute
    1, // Relative
    0, // Implied
};

// precedence levels from lowest to highest
enum prectypes
{
  WHOLEEXP,
  ORORP,
  ANDANDP,
  ORP,
  XORP,
  ANDP,
  EQCOMPARE,
  COMPARE,
  SHIFT,
  PLUSMINUS,
  MULDIV,
  UNARY,
};

enum operators
{
  NOOP,       // no operation
  EQUAL,      // equal to
  NOTEQUAL,   // not equal
  GREATER,    // greater than
  GREATEREQ,  // greater than or equal to
  LESS,       // less than
  LESSEQ,     // less than or equal to
  PLUS,       // addition
  MINUS,      // subtraciton
  MUL,        // multiplication
  DIV,        // division
  MOD,        // modulo
  AND,        // bitwise and
  XOR,        // bitwise exclusive or
  OR,         // bitwise or
  ANDAND,     // boolean and
  OROR,       // boolean or
  LEFTSHIFT,  // bitwise shift left
  RIGHTSHIFT, // bitwise shift right
};

// precedence of each operator
const prectypes prec[] = {
    WHOLEEXP,  // no operation
    EQCOMPARE, // equal to
    EQCOMPARE, // not equal
    COMPARE,   // greater than
    COMPARE,   // greater than or equal to
    COMPARE,   // less than
    COMPARE,   // less than or equal to
    PLUSMINUS, // addition
    PLUSMINUS, // subtraciton
    MULDIV,    // multiplication
    MULDIV,    // division
    MULDIV,    // modulo
    ANDP,      // bitwise and
    XORP,      // bitwise exclusive or
    ORP,       // bitwise or
    ANDANDP,   // boolean and
    ORORP,     // boolean or
    SHIFT,     // bitwise shift left
    SHIFT,     // bitwise shift right
};

struct label
{
  std::string name; //label name

  // value represents different things depending on the label's usage
  // label: memory address,
  // equate: value
  // macro: param count
  // reserved word: function pointer
  ptrdiff_t value;

  int pos; // Location in file; used to determine bank when exporting labels

  char *kitchen_sink; // macros have jank format of *[*next_line, line_text] where
                      // the first <value> lines hold param names.
                      // for opcodes (reserved), this holds opcode definitions, see initlabels

  labeltype type;
  bool used;      // for EQU and MACRO recursion check
  unsigned pass;  // when label was last defined
  unsigned scope; // where this label is visible (0=global, nonzero=local)
  bool ignorenl;  // should this label be suppressed from .nl files
  label *link;    // labels that share the same name are chained together
};

struct instruction
{
  const std::string mnemonic;
  const std::map<optype, byte> opcodes;
};

struct comment
{
  std::string text;
  int pos;
};

typedef void (*directive_func)(label *, std::string::const_iterator &next, const std::string::const_iterator &end);

struct directive
{
  const char *name;
  directive_func func;
};

void addlabel(std::string word, bool force_local);
label *newlabel(const std::string &label_name);
label *findlabel(std::string label_name);

void listline(const std::string &, const std::string &);
void endlist();
void processline(const std::string &src, const std::string &errsrc, const unsigned errline);
void expandmacro(label *, std::string::const_iterator &, const std::string::const_iterator &, unsigned, const std::string &);
void expandrept(int, const char *const);

// reserved word functions //

void inesprg(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void ineschr(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void inesmir(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void inesmap(label *, std::string::const_iterator &next, const std::string::const_iterator &end);

void nes2chrram(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void nes2prgram(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void nes2sub(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void nes2tv(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void nes2vs(label *, std::string::const_iterator &next, const std::string::const_iterator &end) noexcept;
void nes2bram(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void nes2chrbram(label *, std::string::const_iterator &next, const std::string::const_iterator &end);

void align(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void base(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void db(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void dh(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void dl(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void dsb(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void dsw(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void dw(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void else_(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void elseif(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void ende(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void endif(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void endinl(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void endm(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void endr(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void enum_(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void equ(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void equal(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void fillval(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void hex(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void if_(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void ifdef(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void ifndef(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void ignorenl(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void incbin(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void include(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void macro(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void make_error(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void nothing(label *, std::string::const_iterator &next, const std::string::const_iterator &end) noexcept;
void opcode(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void org(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void pad(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void rept(label *, std::string::const_iterator &next, const std::string::const_iterator &end);
void unstable(label *, std::string::const_iterator &next, const std::string::const_iterator &end) noexcept;

#endif