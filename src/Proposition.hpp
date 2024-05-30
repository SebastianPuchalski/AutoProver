#pragma once

#include <memory>

class Proposition {
public:
    virtual ~Proposition() = default;

    enum Type {VARIABLE, CONSTANT, UNARY, BINARY};
    virtual Type getType() const = 0;

    virtual std::shared_ptr<Proposition> copy() const = 0;

    std::shared_ptr<Proposition> toNormalForm(bool cnf) const;
    std::shared_ptr<Proposition> toCNF() const;
    std::shared_ptr<Proposition> toDNF() const;

    /* The following methods operate in-place, modifying the entire proposition tree.
     * They return a shared_ptr to the new proposition if the root is changed, rendering
     * the previous root invalid, which should be removed/replaced by returned one.
     */
    virtual std::shared_ptr<Proposition> transformXnorToImp() = 0;
    virtual std::shared_ptr<Proposition> transformImpToOr() = 0;
    virtual std::shared_ptr<Proposition> eliminateDoubleNot(bool& anyChange) = 0;
    virtual std::shared_ptr<Proposition> moveNotInwardsOp(int binaryOp, bool& anyChange) = 0;
    virtual std::shared_ptr<Proposition> distributeOrAnd(bool orOverAnd, bool& anyChange) = 0; // it may cause redundancy (fix it!)
};
