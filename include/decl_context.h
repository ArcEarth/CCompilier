#pragma once
#include <map>
#include <string>
#include <list>
#include <memory>

namespace std{
	template<>
	class std::less<reference_wrapper<const std::string>>
	{
	public:
		bool operator()(const reference_wrapper<const std::string> & lhs, const reference_wrapper<const std::string>& rhs) const
		{
			return lhs.get() < rhs.get();
		}
	};
}

namespace C1
{
	namespace AST{
		class Declaration;
		class NamedDeclaration;
		class FunctionDeclaration;
		class ValueDeclaration;
		class StructDeclaration;

		// A container specified for declarations , provide a list and lookup table interface
		class DeclContext
		{
		public:
			friend C1::AST::Declaration;
			enum InsertionResult
			{
				Success = 0,
				Success_CompatibleRedefinition = 1,
				Success_IncompatibleRedefinition = 2,
				Failed_Redefinition = 3,
				Failed_IncompatibleRedefinition = 4,
			};

			enum NameCollisonPolicy
			{
				NonRedefinfinable = 0,
				CompatibleRedefinable = 1,
				AlwaysRedefinable = 2,
			};

			typedef std::list<Declaration*>::iterator iterator;
			typedef std::list<Declaration*>::const_iterator const_iterator;
			typedef std::list<Declaration*>::reverse_iterator reverse_iterator;
			typedef std::list<Declaration*>::const_reverse_iterator const_reverse_iterator;

			const DeclContext* parent() const
			{
				return m_ParentContext;
			}
			DeclContext* parent()
			{
				return m_ParentContext;
			}
			~DeclContext();
			void set_parent(DeclContext* val)
			{
				m_ParentContext = val;
			}

			bool empty() const{
				return m_ListView.empty();
			}
			size_t size() const{ return m_ListView.size(); }

			// Lookup table interfaces
			const NamedDeclaration* lookup_local(const std::string& name) const;
			NamedDeclaration* lookup_local(const std::string& name);
			const NamedDeclaration* lookup(const std::string& name) const;
			NamedDeclaration* lookup(const std::string& name);

			// Lookup for a specific type of declaration
			template<typename T>
			const T* lookup(const std::string& name) const;
			template<typename T>
			T* lookup(const std::string& name)
			{
				auto decl = const_cast<const DeclContext*>(this)->lookup<T>(name);
				return const_cast<T*>(decl);
			}
			// Lookup locally for a specific type of declaration
			template<typename T>
			const T* lookup_local(const std::string& name) const;
			template<typename T>
			T* lookup_local(const std::string& name)
			{
				auto decl = const_cast<const DeclContext*>(this)->lookup_local<T>(name);
				return const_cast<T*>(decl);
			}

			const NamedDeclaration* operator[](const std::string& name) const
			{return lookup(name);}
			NamedDeclaration* operator[](const std::string& name)
			{return lookup(name);}

			// List interfaces
			InsertionResult add(Declaration* declaration);

			// Return if the rename operation succeed
			bool rename_decl(NamedDeclaration* decl);

			void force_add(NamedDeclaration* decl);
			void force_add(Declaration* decl);
			//InsertionResult add(NamedDeclaration* declaration, NameCollisonPolicy);
			//InsertionResult add(ValueDeclaration* obj);
			//InsertionResult add(FunctionDeclaration* func);
			//InsertionResult add(StructDeclaration* func);
			//void push_front(Declaration* declaration);
			//void remove(Declaration* declaration);

			// Don't provide non-const access interfaces for safety
			const Declaration* front() const { return m_ListView.front(); }
			const Declaration* back() const { return m_ListView.back(); }
			const_iterator begin() const { return m_ListView.begin(); }
			const_iterator end() const { return m_ListView.end(); }
			const_reverse_iterator rbegin() const { return m_ListView.rbegin(); }
			const_reverse_iterator rend() const { return m_ListView.rend(); }
			//Declaration* front() { return m_ListView.front(); }
			//Declaration* back() { return m_ListView.back(); }
			//iterator begin() { return m_ListView.begin(); }
			//iterator end() { return m_ListView.end(); }
			//reverse_iterator rbegin() { return m_ListView.rbegin(); }
			//reverse_iterator rend() { return m_ListView.rend(); }

		protected:
			DeclContext()
				: m_ParentContext(nullptr), m_pTableView(nullptr)
			{}

			DeclContext(DeclContext* pParentContext)
				: m_ParentContext(pParentContext), m_pTableView(nullptr)
			{}

		private:
			static const size_t IndexizeThreshold = 10;
			void Indexize();

			DeclContext* m_ParentContext;
			// The basic view
			std::list<Declaration*> m_ListView;
			std::unique_ptr<std::multimap<std::reference_wrapper<const std::string>, NamedDeclaration*>> m_pTableView;
		};

		inline size_t total_value_size(const DeclContext& context);

		inline NamedDeclaration* DeclContext::lookup_local(const std::string& name)
		{
			auto decl = const_cast<const DeclContext*>(this)->lookup_local(name);
			return const_cast<NamedDeclaration*>(decl);
		}

		inline NamedDeclaration* DeclContext::lookup(const std::string& name)
		{
			auto decl = const_cast<const DeclContext*>(this)->lookup(name);
			return const_cast<NamedDeclaration*>(decl);
		}

		template<typename T>
		const T* C1::AST::DeclContext::lookup(const std::string& name) const
		{
			const DeclContext* pContext = this;
			while (pContext)
			{
				auto pDecl = pContext->lookup_local<T>(name);
				if (pDecl)
					return pDecl;
				else
					pContext = pContext->parent();
			}
			return nullptr;
		}

		template<typename T>
		const T* C1::AST::DeclContext::lookup_local(const std::string& name) const
		{
			if (m_pTableView)
			{
				auto itr = m_pTableView->find(name);
				if (itr == m_pTableView->end())
					return nullptr;
				else
				{
					while (itr->first.get() == name)
					{
						auto spec_decl = dynamic_cast<const T*>(itr->second);
						if (spec_decl) return spec_decl;
						++itr;
					}
					return nullptr;
				}
			}
			else
			{
				auto itr = std::find_if(m_ListView.rbegin(), m_ListView.rend(),
					[&name](const Declaration* decl)
				{
					auto name_decl = dynamic_cast<const T*>(decl);
					return name_decl && name_decl->Name() == name;
				});
				if (itr == m_ListView.rend())
					return nullptr;
				return static_cast<const T*>(*itr);
			}
		}

	}

}