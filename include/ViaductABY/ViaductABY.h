#ifndef __VIADUCT_ABY_H__
#define __VIADUCT_ABY_H__

#include <vector>
#include <stack>

#include <abycore/aby/abyparty.h>
#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>

enum ABYRole { ABY_SERVER, ABY_CLIENT, ABY_ALL };

struct CircuitBuilders {
  int bitlen;
  ABYRole role;
  ArithmeticCircuit* arith;
  BooleanCircuit* boolean;
  BooleanCircuit* yao;
};

class CircuitGate {
  public:
  virtual void AddChildrenToTraversal(std::vector<CircuitGate*>& children)=0;
  virtual share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders)=0;
};

class DummyInGate: public CircuitGate {
  public:
  DummyInGate();

  void AddChildrenToTraversal(std::vector<CircuitGate*>& children);
  share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders);
};

class InGate: public CircuitGate {
  unsigned int input;
  public:
  InGate(unsigned int input);

  void AddChildrenToTraversal(std::vector<CircuitGate*>& children);

  share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders);
};

class InvGate: public CircuitGate {
  CircuitGate* input;
  public:
  InvGate(CircuitGate* input);

  void AddChildrenToTraversal(std::vector<CircuitGate*>& children);

  share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders);
};

class BinaryOpGate: public CircuitGate {
  protected:
  CircuitGate* lhs;
  CircuitGate* rhs;

  virtual share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders)=0;

  public:

  BinaryOpGate(CircuitGate* lhs, CircuitGate* rhs);

  void AddChildrenToTraversal(std::vector<CircuitGate*>& children);

  share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders);
};

class AddGate: public BinaryOpGate {
  protected:
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  AddGate(CircuitGate* lhs, CircuitGate* rhs);
};

class SubGate: public BinaryOpGate {
  protected:
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  SubGate(CircuitGate* lhs, CircuitGate* rhs);
};

class MulGate: public BinaryOpGate {
  protected: 
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  MulGate(CircuitGate* lhs, CircuitGate* rhs);
};

class ConstGate: public CircuitGate {
  unsigned int value;

  public:
  ConstGate(unsigned int value);

  void AddChildrenToTraversal(std::vector<CircuitGate*>& children);

  share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders);
};

class GtGate: public BinaryOpGate {
  protected: 
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  GtGate(CircuitGate* lhs, CircuitGate* rhs);
};

class AndGate: public BinaryOpGate {
  protected: 
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  AndGate(CircuitGate* lhs, CircuitGate* rhs);
};

class XorGate: public BinaryOpGate {
  protected: 
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  XorGate(CircuitGate* lhs, CircuitGate* rhs);
};

class OrGate: public BinaryOpGate {
  protected: 
  share* BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders);

  public:
  OrGate(CircuitGate* lhs, CircuitGate* rhs);
};

class MuxGate: public CircuitGate {
  private:
  CircuitGate* guard;
  CircuitGate* lhs;
  CircuitGate* rhs;

  public:
  MuxGate(CircuitGate* guard, CircuitGate* lhs, CircuitGate* rhs);

  void AddChildrenToTraversal(std::vector<CircuitGate*>& children);

  share* BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders);
};

/* Builds a representation of a circuit, which can be built into an ABY circuit
 *
 * CachedCircuit is needed because ABY supports building only one circuit at
 * a time, which makes certain Viaduct programs impossible to compile.
 * CachedCircuit represents all possible computations that will be executed,
 * and allows the user to build an ABY circuit whenever some output is needed,
 * bypassing this limitation.
 */
class ViaductABYParty {
  uint32_t bitlen;
  ABYRole role;
  std::vector<CircuitGate*> gates;
  ABYParty party;

public:
  ViaductABYParty(
      ABYRole role,
      char* address, unsigned short port,
      unsigned int bitlen = 32,
      unsigned int secparam = 128,
      unsigned int nthreads = 2
  );

  ~ViaductABYParty();

  CircuitGate* PutDummyINGate();

  CircuitGate* PutINGate(int value);

  CircuitGate* PutCONSTGate(int value);

  CircuitGate* PutADDGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutSUBGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutMULGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutGTGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutINVGate(CircuitGate* input);

  CircuitGate* PutANDGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutXORGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutORGate(CircuitGate* lhs, CircuitGate* rhs);

  CircuitGate* PutMUXGate(CircuitGate* guard, CircuitGate* lhs, CircuitGate* rhs);

  // build an ABY circuit from (some part of) the circuit,
  // ending with the `out` param gate
  //
  // perform iterative traversal instead of recursive traversal because
  // C++ doesn't do tail-call optimization
  int ExecCircuit(CircuitGate* out, ABYRole out_role=ABY_ALL);

  void Reset();
};

#endif
