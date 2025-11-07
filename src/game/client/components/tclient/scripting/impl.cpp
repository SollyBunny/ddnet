#include "impl.h"

#include <base/log.h>

#include <engine/storage.h>

#include <dispatchkit/bad_boxed_cast.hpp>
#include <dispatchkit/boxed_cast.hpp>
#include <dispatchkit/proxy_functions.hpp>
#include <format>
#include <variant>
#include <engine/external/regex.h>

#define CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS_WARNING
#include <chaiscript.hpp>
#include <dispatchkit/boxed_value.hpp>

static const char *ReadScript(IStorage *pStorage, const char *pFilename)
{
	const char *pScript;
	pScript = pStorage->ReadFileStr(pFilename, IStorage::TYPE_SAVE);
	if(!pScript || !*pScript)
		throw std::format("Failed to open script '{}'", pFilename);
	return pScript;
}

class CScriptingCtx::CScriptingCtxData
{
public:
	IStorage *m_pStorage;
	chaiscript::ChaiScript m_Chai;
};

CScriptingCtx::CScriptingCtx()
{
	m_pData = new CScriptingCtxData{
		nullptr,
		chaiscript::ChaiScript({}, {}, {chaiscript::Options::No_Load_Modules, chaiscript::Options::No_External_Scripts})};
	auto PrintStr = chaiscript::fun([&](const std::string &Msg) {
		log_info(SCRIPTING_IMPL "/print", "%s", Msg.c_str());
	});
	m_pData->m_Chai.add(PrintStr, "print_string");
	m_pData->m_Chai.add(PrintStr, "println_string");
	auto PrintStrBoxed = chaiscript::fun([&](const chaiscript::Boxed_Value &Value) {
		chaiscript::Boxed_Value ToStringRaw = m_pData->m_Chai.eval("to_string");
		std::function<std::string(chaiscript::Boxed_Value)> ToString =
			chaiscript::boxed_cast<std::function<std::string(chaiscript::Boxed_Value)>>(ToStringRaw);
		log_info(SCRIPTING_IMPL "/print", "%s", ToString(Value).c_str());
	});
	m_pData->m_Chai.add(PrintStrBoxed, "print");
	m_pData->m_Chai.add(PrintStrBoxed, "puts");
	m_pData->m_Chai.add(chaiscript::fun([&](const std::string &Module) {
		const char *pScript = ReadScript(m_pData->m_pStorage, Module.c_str());
		auto Out = m_pData->m_Chai.eval(pScript, chaiscript::Exception_Handler(), Module);
		std::free((void *)pScript);
		return Out;
	}),
		"include");
	m_pData->m_Chai.add(chaiscript::fun([&](const std::string &Path) {
		return m_pData->m_pStorage->FileExists(Path.c_str(), IStorage::TYPE_SAVE);
	}),
		"file_exists");
	m_pData->m_Chai.add(chaiscript::fun([&](const std::string &Pattern, const std::string &String) {
		auto Re = Regex(Pattern);
		if(Re.error().empty())
			return Re.match(String.c_str());
		else
			throw std::runtime_error(Re.error());
	}),
		"re_match");
}

CScriptingCtx::~CScriptingCtx()
{
	delete m_pData;
}

static chaiscript::Boxed_Value Any2Boxed(const CScriptingCtx::Any &Any)
{
	if(std::holds_alternative<std::nullptr_t>(Any))
		return chaiscript::void_var();
	if(std::holds_alternative<std::string>(Any))
		return chaiscript::const_var(std::get<std::string>(Any));
	if(std::holds_alternative<bool>(Any))
		return chaiscript::const_var(std::get<bool>(Any));
	if(std::holds_alternative<int>(Any))
		return chaiscript::const_var(std::get<int>(Any));
	if(std::holds_alternative<float>(Any))
		return chaiscript::const_var(std::get<float>(Any));
	throw chaiscript::exception::bad_boxed_cast("Cannot convert Any to Boxed_Value");
}

template<>
void CScriptingCtx::AddFunction(const char *pName, const std::function<CScriptingCtx::Any(const std::string &Str)> &Function)
{
	m_pData->m_Chai.add(chaiscript::fun([&](const std::string &Str) {
		return Any2Boxed(Function(Str));
	}),
		pName);
}

template<>
void CScriptingCtx::AddFunction(const char *pName, const std::function<void(const std::string &Str)> &Function)
{
	m_pData->m_Chai.add(chaiscript::fun(Function), pName);
}

template<>
void CScriptingCtx::AddGlobal(const char *pName, const std::string &Object)
{
	m_pData->m_Chai.add_global_const(chaiscript::const_var(Object), pName);
}

void CScriptingCtx::Run(IStorage *pStorage, const char *pFilename, const char *pArgs)
{
	m_pData->m_pStorage = pStorage;
	const char *pScript = nullptr;
	try
	{
		m_pData->m_Chai.add_global_const(chaiscript::const_var(std::string(pArgs)), "args");
		pScript = ReadScript(pStorage, pFilename);
		m_pData->m_Chai.eval(pScript, chaiscript::Exception_Handler(), pFilename);
	}
	catch(const chaiscript::exception::eval_error &e)
	{
		log_error(SCRIPTING_IMPL, "Eval error in '%s': %s", pFilename, e.pretty_print().c_str());
	}
	catch(const std::exception &e)
	{
		log_error(SCRIPTING_IMPL, "Exception in '%s': %s", pFilename, e.what());
	}
	catch(const std::string &e)
	{
		log_error(SCRIPTING_IMPL, "Exception in '%s': %s", pFilename, e.c_str());
	}
	catch(...)
	{
		log_error(SCRIPTING_IMPL, "Unknown exception in '%s'", pFilename);
	}
	std::free((void *)pScript);
}
