import sys
from .datatypes import emit_definition
from . import content  # pylint: disable=no-name-in-module
from . import network  # pylint: disable=no-name-in-module


def create_enum_table(names, num):
	lines = []
	print("enum", "{")
	print(f"	{names[0]}=0,")
	for name in names[1:]:
		print(f"	{name},")
	print(f"	{num}", "};")
	return lines


def create_flags_table(names):
	lines = []
	print("enum", "{")
	i = 0
	for name in names:
		print(f"	{name} = 1<<{int(i)},")
		i += 1
	print("};")
	return lines


def emit_enum(names, num):
	print("enum")
	print("{")
	print(f"	{names[0]}=0,")
	for name in names[1:]:
		print(f"	{name},")
	print(f"	{num}")
	print("};")


def emit_flags(names):
	print("enum")
	print("{")
	i = 0
	for name in names:
		print(f"	{name} = 1<<{int(i)},")
		i += 1
	print("};")


def main():
	gen_network_header = "network_header" in sys.argv
	gen_network_source = "network_source" in sys.argv
	gen_client_content_header = "client_content_header" in sys.argv
	gen_client_content_source = "client_content_source" in sys.argv
	gen_server_content_header = "server_content_header" in sys.argv
	gen_server_content_source = "server_content_source" in sys.argv

	if gen_client_content_header:
		print("#ifndef CLIENT_CONTENT7_HEADER")
		print("#define CLIENT_CONTENT7_HEADER")

	if gen_server_content_header:
		print("#ifndef SERVER_CONTENT7_HEADER")
		print("#define SERVER_CONTENT7_HEADER")

	if gen_client_content_header or gen_server_content_header:
		# print some includes
		print("#include <engine/graphics.h>")
		print("#include \"data_types.h\"")
		print("namespace client_data7 {")

		# the container pointer
		print("extern CDataContainer *g_pData;")

		# enums
		emit_enum([f"IMAGE_{i.name.value.upper()}" for i in content.container.images.items], "NUM_IMAGES")
		emit_enum([f"ANIM_{i.name.value.upper()}" for i in content.container.animations.items], "NUM_ANIMS")
		emit_enum([f"SPRITE_{i.name.value.upper()}" for i in content.container.sprites.items], "NUM_SPRITES")

	if gen_client_content_source or gen_server_content_source:
		if gen_client_content_source:
			print("#include \"client_data7.h\"")
		if gen_server_content_source:
			print("#include \"server_data.h\"")
		print("namespace client_data7 {")
		emit_definition(content.container, "datacontainer")
		print("CDataContainer *g_pData = &datacontainer;")
		print("}")

# NETWORK
	if gen_network_header:

		print("#ifndef GAME_GENERATED_PROTOCOL7_H")
		print("#define GAME_GENERATED_PROTOCOL7_H")
		print("class CUnpacker;")
		print("#include <engine/message.h>")
		print("namespace protocol7 {")
		print(network.RawHeader)

		for flag in network.Enums:
			for line in create_enum_table([f"{flag.name}_{v}" for v in flag.values], f"NUM_{flag.name}S"):
				print(line)
			print("")

		for flag in network.Flags:
			for line in create_flags_table([f"{flag.name}_{value}" for value in flag.values]):
				print(line)
			print("")

		for line in create_enum_table(["NETOBJ_INVALID"] + [obj.enum_name for obj in network.Objects], "NUM_NETOBJTYPES"):
			print(line)
		print("")
		for line in create_enum_table(["NETMSG_INVALID"] + [obj.enum_name for obj in network.Messages], "NUM_NETMSGTYPES"):
			print(line)
		print("")

		print("""
	template<typename... Ts> struct make_void { typedef void type;};
	template<typename... Ts> using void_t = typename make_void<Ts...>::type;

	template<typename T, typename = void>
	struct is_sixup {
		constexpr static bool value = false;
	};

	template<typename T>
	struct is_sixup<T, void_t<typename T::is_sixup>> {
		constexpr static bool value = true;
	};
	""")

		for item in network.Objects + network.Messages:
			for line in item.emit_declaration():
				print(line)
			print("")

		emit_enum([f"SOUND_{i.name.value.upper()}" for i in content.container.sounds.items], "NUM_SOUNDS")
		emit_enum([f"WEAPON_{i.name.value.upper()}" for i in content.container.weapons.id.items], "NUM_WEAPONS")

		print("""

	class CNetObjHandler
	{
		const char *m_pMsgFailedOn;
		char m_aMsgData[1024];
		const char *m_pObjFailedOn;
		int m_NumObjFailures;
		bool CheckInt(const char *pErrorMsg, int Value, int Min, int Max);
		bool CheckFlag(const char *pErrorMsg, int Value, int Mask);

		static const char *ms_apObjNames[];
		static int ms_aObjSizes[];
		static const char *ms_apMsgNames[];

	public:
		CNetObjHandler();

		int ValidateObj(int Type, const void *pData, int Size);
		const char *GetObjName(int Type) const;
		int GetObjSize(int Type) const;
		const char *FailedObjOn() const;
		int NumObjFailures() const;

		const char *GetMsgName(int Type) const;
		void *SecureUnpackMsg(int Type, CUnpacker *pUnpacker);
		const char *FailedMsgOn() const;
	};

	""")

		print("}")
		print("#endif // GAME_GENERATED_PROTOCOL7_H")

	if gen_network_source:
		# create names
		print("""#include "protocol7.h"

#include <base/system.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>

namespace protocol7 {
	CNetObjHandler::CNetObjHandler()
	{
		m_pMsgFailedOn = "";
		m_pObjFailedOn = "";
		m_NumObjFailures = 0;
	}

	const char *CNetObjHandler::FailedObjOn() const { return m_pObjFailedOn; }
	int CNetObjHandler::NumObjFailures() const { return m_NumObjFailures; }
	const char *CNetObjHandler::FailedMsgOn() const { return m_pMsgFailedOn; }

	static const int max_int = 0x7fffffff;

	bool CNetObjHandler::CheckInt(const char *pErrorMsg, int Value, int Min, int Max)
	{
		if(Value < Min || Value > Max)
		{
			m_pObjFailedOn = pErrorMsg;
			m_NumObjFailures++;
			return false;
		}
		return true;
	}

	bool CNetObjHandler::CheckFlag(const char *pErrorMsg, int Value, int Mask)
	{
		if((Value&Mask) != Value)
		{
			m_pObjFailedOn = pErrorMsg;
			m_NumObjFailures++;
			return false;
		}
		return true;
	}
""")

		print("const char *CNetObjHandler::ms_apObjNames[] = {")
		for obj in network.Objects:
			print(f"\t\"{obj.name}\",")
		print("\t\"\"")
		print("};")

		print("int CNetObjHandler::ms_aObjSizes[] = {")
		for obj in network.Objects:
			print(f"\tsizeof({obj.struct_name}),")
		print("\t0")
		print("};")

		print("const char *CNetObjHandler::ms_apMsgNames[] = {")
		print("\t\"invalid\",")
		for msg in network.Messages:
			print(f"\t\"{msg.name}\",")
		print("\t\"\"")
		print("};")

		print("""
const char *CNetObjHandler::GetObjName(int Type) const
{
	if(Type < 0 || Type >= NUM_NETOBJTYPES)
		return "(out of range)";
	return ms_apObjNames[Type];
}

int CNetObjHandler::GetObjSize(int Type) const
{
	if(Type < 0 || Type >= NUM_NETOBJTYPES)
		return 0;
	return ms_aObjSizes[Type];
};

const char *CNetObjHandler::GetMsgName(int Type) const
{
	if(Type < 0 || Type >= NUM_NETMSGTYPES)
		return "(out of range)";
	return ms_apMsgNames[Type];
};
""")

		print("int CNetObjHandler::ValidateObj(int Type, const void *pData, int Size)")
		print("{")
		print("\tswitch(Type)")
		print("\t{")
		for item in network.Objects:
			for line in item.emit_validate(network.Objects):
				print("\t" + line)
			print("\t")
		print("\t}")
		print("\treturn -1;")
		print("};")
		print("")

		print("void *CNetObjHandler::SecureUnpackMsg(int Type, CUnpacker *pUnpacker)")
		print("{")
		print("\tm_pMsgFailedOn = nullptr;")
		print("\tm_pObjFailedOn = nullptr;")
		print("\tswitch(Type)")
		print("\t{")

		for item in network.Messages:
			for line in item.emit_unpack():
				print("\t" + line)
			print("\t")

		print("\tdefault:")
		print("\t\tm_pMsgFailedOn = \"(type out of range)\";")
		print("\t\tbreak;")
		print("\t}")
		print("\t")
		print("\tif(pUnpacker->Error())")
		print("\t\tm_pMsgFailedOn = \"(unpack error)\";")
		print("\t")
		print("\tif(m_pMsgFailedOn || m_pObjFailedOn) {")
		print("\t\tif(!m_pMsgFailedOn)")
		print("\t\t\tm_pMsgFailedOn = \"\";")
		print("\t\tif(!m_pObjFailedOn)")
		print("\t\t\tm_pObjFailedOn = \"\";")
		print("\t\treturn nullptr;")
		print("\t}")
		print("\tm_pMsgFailedOn = \"\";")
		print("\tm_pObjFailedOn = \"\";")
		print("\treturn m_aMsgData;")
		print("};")
		print("}")
		print("")

	if gen_client_content_header or gen_server_content_header:
		print("}")
		print("#endif")


if __name__ == '__main__':
	main()
