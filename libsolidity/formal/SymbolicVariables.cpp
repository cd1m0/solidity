/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libsolidity/formal/SymbolicVariables.h>

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicVariable::SymbolicVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	m_type(move(_type)),
	m_uniqueName(_uniqueName),
	m_interface(_interface),
	m_ssa(make_shared<SSAVariable>())
{
}

string SymbolicVariable::currentName() const
{
	return uniqueSymbol(m_ssa->index());
}

string SymbolicVariable::uniqueSymbol(unsigned _index) const
{
	return m_uniqueName + "_" + to_string(_index);
}

SymbolicMappingVariable::SymbolicMappingVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), _uniqueName, _interface)
{
	solAssert(isMapping(m_type->category()), "");
}

smt::Expression SymbolicMappingVariable::valueAtIndex(int _index) const
{
	auto mapType = dynamic_cast<MappingType const*>(m_type.get());
	solAssert(mapType, "");
	auto domain = smtSort(*mapType->keyType());
	auto range = smtSort(*mapType->valueType());
	return m_interface.newVariable(uniqueSymbol(_index), make_shared<smt::ArraySort>(domain, range));
}

SymbolicBoolVariable::SymbolicBoolVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface&_interface
):
	SymbolicVariable(move(_type), _uniqueName, _interface)
{
	solAssert(m_type->category() == Type::Category::Bool, "");
}

smt::Expression SymbolicBoolVariable::valueAtIndex(int _index) const
{
	return m_interface.newVariable(uniqueSymbol(_index), make_shared<smt::Sort>(smt::Kind::Bool));
}

SymbolicIntVariable::SymbolicIntVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), _uniqueName, _interface)
{
	solAssert(isNumber(m_type->category()), "");
}

smt::Expression SymbolicIntVariable::valueAtIndex(int _index) const
{
	return m_interface.newVariable(uniqueSymbol(_index), make_shared<smt::Sort>(smt::Kind::Int));
}

SymbolicFunctionDeclaration::SymbolicFunctionDeclaration(
	smt::SortPointer const& _sort,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	m_declaration(_interface.newVariable(_uniqueName, _sort))
{
}

SymbolicFunctionDeclaration::SymbolicFunctionDeclaration(
	FunctionTypePointer const& _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	m_declaration(_interface.newVariable(_uniqueName, smtSort(*_type)))
{
}

smt::Expression SymbolicFunctionDeclaration::operator()(vector<smt::Expression> _arguments)
{
	return m_declaration(std::move(_arguments));
}
