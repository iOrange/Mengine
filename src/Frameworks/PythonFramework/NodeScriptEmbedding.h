#pragma once

#include "Interface/ScriptEmbeddingInterface.h"

#include "Kernel/Factorable.h"

namespace Mengine
{
    class NodeScriptEmbedding
        : public ScriptEmbeddingInterface
        , public Factorable
    {
    public:
        NodeScriptEmbedding();
        ~NodeScriptEmbedding() override;

    public:
        bool embedding() override;
        void ejecting() override;

    protected:
        FactorablePtr m_implement;
    };
}