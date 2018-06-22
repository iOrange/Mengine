#pragma once

#include "Interface/PrototypeManagerInterface.h"

#include "Core/ServiceBase.h"
#include "Core/ConstString.h"

#include "Config/Vector.h"

#ifndef MENGINE_PROTOTYPE_HASH_SIZE
#   define MENGINE_PROTOTYPE_HASH_SIZE 257
#endif

namespace Mengine
{
	class PrototypeManager
        : public ServiceBase<PrototypeServiceInterface>
	{
	public:
		PrototypeManager();
		~PrototypeManager() override;

	public:
		bool _initializeService() override;
		void _finalizeService() override;

	public:
		bool addPrototype( const ConstString & _category, const ConstString & _prototype, const PrototypeGeneratorInterfacePtr & _generator ) override;
        bool removePrototype( const ConstString & _category, const ConstString & _prototype ) override;
		bool hasPrototype( const ConstString & _category, const ConstString & _prototype, PrototypeGeneratorInterfacePtr & _generator ) const override;

    public:
        PointerFactorable generatePrototype( const ConstString & _category, const ConstString & _prototype ) override;

    public:
        void visitGenerators( VisitorPrototypeGenerator * _visitor ) const override;

	protected:
        struct CategoryKey
        {
            ConstString category;
            ConstString prototype;
            PrototypeGeneratorInterfacePtr generator;
        };

        typedef Vector<CategoryKey> TVectorPrototypes;
        TVectorPrototypes m_prototypes[MENGINE_PROTOTYPE_HASH_SIZE];
	};
}