#include <vector>
#include <stack>

#include <abycore/aby/abyparty.h>
#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>

#include <ENCRYPTO_utils/crypto/crypto.h>

#include <ViaductABY/ViaductABY.h>

#define CAST_ROLE(role) (role == ABY_CLIENT ? CLIENT : (role == ABY_SERVER ? SERVER : ALL))


// DummyInGate
DummyInGate::DummyInGate() {};

void DummyInGate::AddChildrenToTraversal(std::vector<CircuitGate*>& children) {}

share* DummyInGate::BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders) {
  return builders.yao->PutDummyINGate(builders.bitlen);
}

// InGate
InGate::InGate(uint32_t input) {
  this->input = input;
}

void InGate::AddChildrenToTraversal(std::vector<CircuitGate*>& children) {}

share* InGate::BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders) {
  return builders.yao->PutINGate(input, builders.bitlen, CAST_ROLE(builders.role));
}

// InvGate
InvGate::InvGate(CircuitGate* input) {
  this->input = input;
}

void InvGate::AddChildrenToTraversal(std::vector<CircuitGate*>& children) {
  children.push_back(this->input);
}

share* InvGate::BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders) {
  share* inputShare = shareStack.top();
  return builders.boolean->PutINVGate(inputShare);
}

// BinaryOpGate
BinaryOpGate::BinaryOpGate(CircuitGate* lhs, CircuitGate* rhs) {
  this->lhs = lhs;
  this->rhs = rhs;
}

void BinaryOpGate::AddChildrenToTraversal(std::vector<CircuitGate*>& children) {
  children.push_back(this->lhs);
  children.push_back(this->rhs);
}

share* BinaryOpGate::BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders) {
  share* lhsShare = shareStack.top();
  shareStack.pop();
  share* rhsShare = shareStack.top();
  shareStack.pop();
  return BuildGate(lhsShare, rhsShare, builders);
}

// AddGate
share* AddGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutADDGate(lhsShare, rhsShare);
}

AddGate::AddGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// SubGate
share* SubGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutSUBGate(lhsShare, rhsShare);
}

SubGate::SubGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// MulGate
share* MulGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutMULGate(lhsShare, rhsShare);
}

MulGate::MulGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// ConstGate
ConstGate::ConstGate(uint32_t value) { this->value = value; }

void ConstGate::AddChildrenToTraversal(std::vector<CircuitGate*>& children) {}

share* ConstGate::BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders) {
  return builders.yao->PutCONSGate(this->value, builders.bitlen);
}

// GtGate
share* GtGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutGTGate(lhsShare, rhsShare);
}

GtGate::GtGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// AndGate
share* AndGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutANDGate(lhsShare, rhsShare);
}

AndGate::AndGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// XorGate
share* XorGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutXORGate(lhsShare, rhsShare);
}

XorGate::XorGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// OrGate
share* OrGate::BuildGate(share* lhsShare, share* rhsShare, CircuitBuilders& builders) {
  return builders.yao->PutORGate(lhsShare, rhsShare);
}

OrGate::OrGate(CircuitGate* lhs, CircuitGate* rhs) : BinaryOpGate(lhs, rhs) {}

// MuxGate
MuxGate::MuxGate(CircuitGate* guard, CircuitGate* lhs, CircuitGate* rhs) {
  this->guard = guard;
  this->lhs = lhs;
  this->rhs = rhs;
}

void MuxGate::AddChildrenToTraversal(std::vector<CircuitGate*>& children) {
  children.push_back(this->guard);
  children.push_back(this->lhs);
  children.push_back(this->rhs);
}

share* MuxGate::BuildGate(std::stack<share*>& shareStack, CircuitBuilders& builders) {
  share* guardShare = shareStack.top();
  shareStack.pop();
  share* lhsShare = shareStack.top();
  shareStack.pop();
  share* rhsShare = shareStack.top();
  shareStack.pop();
  return builders.yao->PutMUXGate(guardShare, lhsShare, rhsShare);
}



/* Builds a representation of a circuit, which can be built into an ABY circuit
 *
 * CachedCircuit is needed because ABY supports building only one circuit at
 * a time, which makes certain Viaduct programs impossible to compile.
 * CachedCircuit represents all possible computations that will be executed,
 * and allows the user to build an ABY circuit whenever some output is needed,
 * bypassing this limitation.
 */
ViaductABYParty::ViaductABYParty(
  ABYRole role,
  char* address, unsigned short port,
  unsigned int bitlen,
  unsigned int secparam,
  unsigned int nthreads
) : role(role), bitlen(bitlen), gates(),
    party(role == ABY_CLIENT ? CLIENT : SERVER, std::string(address), port, get_sec_lvl(secparam), nthreads) {}

ViaductABYParty::~ViaductABYParty() {
  for (uint32_t i = 0; i < this->gates.size(); i++) {
    delete this->gates[i];
  }
}

CircuitGate* ViaductABYParty::PutDummyINGate() {
  DummyInGate* gate = new DummyInGate();
  this->gates.push_back(gate);
  return gate;
}

CircuitGate* ViaductABYParty::PutINGate(int value) {
  InGate* gate = new InGate(value);
  this->gates.push_back(gate);
  return gate;
}

CircuitGate* ViaductABYParty::PutCONSTGate(int value) {
  ConstGate* gate = new ConstGate(value);
  this->gates.push_back(gate);
  return gate;
}

CircuitGate* ViaductABYParty::PutADDGate(CircuitGate* lhs, CircuitGate* rhs) {
  AddGate* gate = new AddGate(lhs, rhs);
  this->gates.push_back(gate);
  return gate;
}

CircuitGate* ViaductABYParty::PutMULGate(CircuitGate* lhs, CircuitGate* rhs) {
  MulGate* gate = new MulGate(lhs, rhs);
  this->gates.push_back(gate);
  return gate;
}

CircuitGate* ViaductABYParty::PutGTGate(CircuitGate* lhs, CircuitGate* rhs) {
  GtGate* gate = new GtGate(lhs, rhs);
  this->gates.push_back(gate);
  return gate;
}

/*
CircuitGate* PutINVGate(CircuitGate* input) {
  InvGate* gate = new InvGate(input);
  this->gates.push_back(gate);
  return gate;
}
*/

// build an ABY circuit from (some part of) the circuit,
// ending with the `out` param gate
//
// perform iterative traversal instead of recursive traversal because
// C++ doesn't do tail-call optimization
int ViaductABYParty::ExecCircuit(CircuitGate* out, ABYRole out_role) {
  CircuitBuilders builders;
  std::vector<Sharing*>& sharings = this->party.GetSharings();
  builders.arith = (ArithmeticCircuit*)sharings[S_ARITH]->GetCircuitBuildRoutine();
  builders.boolean = (BooleanCircuit*)sharings[S_BOOL]->GetCircuitBuildRoutine();
  builders.yao = (BooleanCircuit*)sharings[S_YAO]->GetCircuitBuildRoutine();
  builders.bitlen = this->bitlen;
  builders.role = this->role;

  // use this as an pre-order traversal
  std::stack<CircuitGate*> traverseStack = std::stack<CircuitGate*>();

  // contains postorder traversal of the expr tree
  std::stack<CircuitGate*> exprStack = std::stack<CircuitGate*>();

  std::stack<share*> shareStack = std::stack<share*>();
  traverseStack.push(out);

  while (!traverseStack.empty()) {
    CircuitGate* curGate = traverseStack.top();
    traverseStack.pop();
    exprStack.push(curGate);

    std::vector<CircuitGate*> children = std::vector<CircuitGate*>();
    curGate->AddChildrenToTraversal(children);
    for (uint32_t i = 0; i < children.size(); i++) {
      traverseStack.push(children[i]);
    }
  }

  // "evaluates" the stack as an reverse Polish expr by building the circuit
  while (!exprStack.empty()) {
    CircuitGate* curGate = exprStack.top();
    exprStack.pop();
    share* curShare = curGate->BuildGate(shareStack, builders);
    shareStack.push(curShare);
  }

  // at the end, the only node in the stack should be the top-level expr node
  assert(shareStack.size() == 1);

  share* out_share = builders.yao->PutOUTGate(shareStack.top(), CAST_ROLE(out_role));
  this->party.ExecCircuit();

  if (out_role == ABY_ALL || out_role == role) {
    return out_share->get_clear_value<int>();

  } else {
    return 0;
  }
}

void ViaductABYParty::Reset() {
  this->party.Reset();
}