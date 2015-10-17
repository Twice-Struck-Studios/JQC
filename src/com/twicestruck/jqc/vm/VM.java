package com.twicestruck.jqc.vm;

import java.util.*;
import java.io.*;
import java.nio.file.*;

public class VM
{
	//private fields
	private static final int OFS_RETURN = 1;
	private static final int OFS_PARM0 = 4;
	private static final int OFS_PARM1 = 7;
	private static final int OFS_PARM2 = 10;
	private static final int OFS_PARM3 = 13;
	private static final int OFS_PARM4 = 16;
	private static final int OFS_PARM5 = 19;
	private static final int OFS_PARM6 = 22;
	private static final int OFS_PARM7 = 25;

	private String filename;
;
	private int vmSize;
	private byte[] vmData;

	private Header header;
	private Statement[] statements;
	private Definition[] globalDefs;
	private Definition[] fieldDefs;
	private Function[] functions;
	private byte[] stringData;
	private float[] globalData;

	private static final int GLOBALDEF_TYPE_MASK = 0x07;
	private static final byte GLOBAL_DEF_SYSTEM = 1 << 0;
	private static final byte GLOBAL_DEF_FIELD = 1 << 1;
	private static final byte GLOBAL_DEF_LOCAL = 1 << 2;
	private static final byte GLOBAL_DEF_SPECIAL = 1 << 3;
	private byte[] globalDefData;

	private DefinitionType[] fieldOffsetTypes;

	private StringManager stringManager;
	private EntityManager entityManager;
	
	private HashMap<Integer, BuiltinCallback> builtins;
	private int numCallParameters;
	
	//public fields
	public Object dataObject;
	public static final float ENTITY_REUSE_DELAY = 2.0f;
	
	//constructor
	public VM(String filename)
	{
		this.filename = filename;
		
		vmSize = 0;
		vmData = null;
		header = null;
		statements = null;
		globalDefs = null;
		fieldDefs = null;
		functions = null;
		stringData = null;
		globalData = null;
		
		globalDefData = null;
		fieldOffsetTypes = null;
		dataObject = null;
		
		load();
	}
	
	//public methods
	public boolean isLoaded()
	{
		return vmData != null;
	}
	
	public int getCRC()
	{
		if(!isLoaded())
		{
			return 0;
		}
		return header.crc;
	}
	
	public int getNumCallParameters()
	{
		return numCallParameters;
	}
	
	public void dump()
	{
		//TODO implement this
	}
	
	//private methods
	private String nameForGlobalOffset(int ofs)
	{
		String s = "?";
		for(int i = 0; i < header.globaldefs_num; ++i)
		{
			if(globalDefs[i].offset == ofs)
			{
				s = "";
				for(int j = 0; (char)stringData[globalDefs[i].nameOffset+j] != '\0'; ++j)
				{
					s += (char)stringData[globalDefs[i].nameOffset+j];
				}
				i = header.globaldefs_num;
			}
		}
		return s;
	}

	private String nameForGlobalOffset(int ofs, DefinitionType type)
	{
		String s = "?";
		for(int i = 0; i < header.globaldefs_num; ++i)
		{
			if((globalDefs[i].type & GLOBALDEF_TYPE_MASK) == type.t && globalDefs[i].offset == ofs)
			{
				s = "";
				for(int j = 0; (char)stringData[globalDefs[i].nameOffset+j] != '\0'; ++j)
				{
					s += (char)stringData[globalDefs[i].nameOffset+j];
				}
				i = header.globaldefs_num;
			}
		}
		return s;
	}

	private String nameForFieldOffset(int ofs)
	{
		String s = "?";
		for(int i = 0; i < header.fielddefs_num; ++i)
		{
			if(fieldDefs[i].offset == ofs)
			{
				s = "";
				for(int j = 0; (char)stringData[fieldDefs[i].nameOffset+j] != '\0'; ++j)
				{
					s += (char)stringData[fieldDefs[i].nameOffset+j];
				}
				i = header.fielddefs_num;
			}
		}
		return s;
	}

	private String nameForFieldOffset(int ofs, DefinitionType type)
	{
		String s = "?";
		for(int i = 0; i < header.fielddefs_num; ++i)
		{
			if((fieldDefs[i].type & GLOBALDEF_TYPE_MASK) == type.t && fieldDefs[i].offset == ofs)
			{
				s = "";
				for(int j = 0; (char)stringData[fieldDefs[i].nameOffset+j] != '\0'; ++j)
				{
					s += (char)stringData[fieldDefs[i].nameOffset+j];
				}
				i = header.fielddefs_num;
			}
		}
		return s;
	}
	
	private int READ_INT(byte[] data, int offset)
	{
		return ((int)data[offset] << 24) | ((int)data[offset+1] << 16) | ((int)data[offset+2] << 8) | (int)data[offset+3]; 
	}
	
	private float READ_FLOAT(byte[] data, int offset)
	{
		return Float.intBitsToFloat(READ_INT(data, offset)); 
	}
	
	private short READ_SHORT(byte[] data, int offset)
	{
		return (short)(((short)data[offset] << 8) | (short)data[offset+1]);
	}
	
	private void BOUNDS_CHECK_GLOBAL(int i, int n)
	{
		if(statements[i].parameter[n] < 0 || statements[i].parameter[n] >= header.globaldata_num)
		{
			throw new Error.OffsetOutOfBounds("Instruction " + i + " parameter", n, filename);
		}
	}
	
	private void load()
	{
		ArrayList<Byte> data = new ArrayList<Byte>();
		try
		{
			InputStream progsFile = Files.newInputStream(FileSystems.getDefault().getPath(filename));
			while(progsFile.available() > 0)
			{
				data.add((byte)progsFile.read());
			}
		}
		catch(Exception e)
		{
			System.out.println("Error reading");
			System.exit(1);
		}
		vmData = new byte[data.size()];
		for(int i = 0; i < data.size(); ++i)
		{
			vmData[i] = data.get(i);
		}
		vmSize = vmData.length;
		
		header = new Header();
		
		if(vmSize < header.getClass().getFields().length*4)
		{
			throw new Error.ProgsSmallerThanHeader(vmSize, header.getClass().getFields().length*4);
		}
		
		header.version = READ_INT(vmData, 0);
		header.crc = READ_INT(vmData, 4);
		header.statements_offset = READ_INT(vmData, 8);
		header.statements_num = READ_INT(vmData, 12);
		header.globaldefs_offset = READ_INT(vmData, 16);
		header.globaldefs_num = READ_INT(vmData, 20);
		header.fielddefs_offset = READ_INT(vmData, 24);
		header.fielddefs_num = READ_INT(vmData, 28);
		header.functions_offset = READ_INT(vmData, 32);
		header.functions_num = READ_INT(vmData, 36);
		header.stringdata_offset = READ_INT(vmData, 40);
		header.stringdata_size = READ_INT(vmData, 44);
		header.globaldata_offset = READ_INT(vmData, 48);
		header.globaldata_num = READ_INT(vmData, 52);
		header.entity_size = READ_INT(vmData, 56);
		
		if(header.version != 6)
		{
			throw new Error.ProgsInvalidVersion(header.version, 6);
		}
		
		if(header.statements_offset + header.statements_num*Statement.SIZEOF > vmSize || header.statements_offset < 0)
		{
			throw new Error.ProgsSmallerThanLump("Statement", filename);
		}
		statements = new Statement[header.statements_num];
		for(int i = 0; i < header.statements_num; ++i)
		{
			statements[i].instruction = READ_SHORT(vmData, header.statements_offset + i * Statement.SIZEOF);
			statements[i].parameter = new short[3];
			statements[i].parameter[0] = READ_SHORT(vmData, header.statements_offset + 2 + i * Statement.SIZEOF);
			statements[i].parameter[1] = READ_SHORT(vmData, header.statements_offset + 4 + i * Statement.SIZEOF);
			statements[i].parameter[2] = READ_SHORT(vmData, header.statements_offset + 6 + i * Statement.SIZEOF);
		}
		
		if(header.globaldefs_offset + header.globaldefs_num*Definition.SIZEOF > vmSize || header.globaldefs_offset < 0)
		{
			throw new Error.ProgsSmallerThanLump("Globaldef", filename);
		}
		globalDefs = new Definition[header.globaldefs_num];
		for(int i = 0; i < header.globaldefs_num; ++i)
		{
			globalDefs[i].type = READ_SHORT(vmData, header.globaldefs_offset + i * Definition.SIZEOF);
			globalDefs[i].offset = READ_SHORT(vmData, header.globaldefs_offset + 2 + i * Definition.SIZEOF);
			globalDefs[i].nameOffset = READ_INT(vmData, header.globaldefs_offset + 4 + i *  Definition.SIZEOF);
		}
		
		if(header.fielddefs_offset + header.fielddefs_num*Definition.SIZEOF > vmSize || header.fielddefs_offset < 0)
		{
			throw new Error.ProgsSmallerThanLump("Fielddef", filename);
		}
		fieldDefs = new Definition[header.fielddefs_num];
		for(int i = 0; i < header.fielddefs_num; ++i)
		{
			fieldDefs[i].type = READ_SHORT(vmData, header.fielddefs_offset + i * Definition.SIZEOF);
			fieldDefs[i].offset = READ_SHORT(vmData, header.fielddefs_offset + 2 + i * Definition.SIZEOF);
			fieldDefs[i].nameOffset = READ_INT(vmData, header.fielddefs_offset + 4 + i * Definition.SIZEOF);
		}
		
		if(header.functions_offset + header.functions_num*Function.SIZEOF > vmSize || header.functions_offset < 0)
		{
			throw new Error.ProgsSmallerThanLump("Function", filename);
		}
		functions = new Function[header.functions_num];
		for(int i = 0; i < header.functions_num; ++i)
		{
			functions[i].offsetFirstStatement = READ_INT(vmData, header.functions_offset + i * Function.SIZEOF);
			functions[i].offsetLocalsInGlobals = READ_INT(vmData, header.functions_offset + 4 + i * Function.SIZEOF);
			functions[i].numLocals = READ_INT(vmData, header.functions_offset + 8 + i * Function.SIZEOF);
			functions[i].profiling = READ_INT(vmData, header.functions_offset + 12 + i * Function.SIZEOF);
			functions[i].nameOffset = READ_INT(vmData, header.functions_offset + 16 + i * Function.SIZEOF);
			functions[i].fileNameOffset = READ_INT(vmData, header.functions_offset + 20 + i * Function.SIZEOF);
			functions[i].numParameters = READ_INT(vmData, header.functions_offset + 24 + i * Function.SIZEOF);
			for(int j = 0; j < functions[i].parameterSizes.length; ++j)
			{
				functions[i].parameterSizes[j] = vmData[header.functions_offset + 28 + i * Function.SIZEOF + j];
			}
		}
		
		if(header.stringdata_offset + header.stringdata_size/* * sizeof(byte)*/ > vmSize || header.stringdata_offset < 0)
		{
			throw new Error.ProgsSmallerThanLump("Stringdata", filename);
		}
		stringData = new byte[header.stringdata_size];
		for(int i = 0; i < header.stringdata_size; ++i)
		{
			stringData[i] = vmData[header.stringdata_offset + i];
		}
		
		if(header.globaldata_offset + header.globaldata_num * 4 /* * sizeof(float)*/ > vmSize || header.globaldata_offset < 0)
		{
			throw new Error.ProgsSmallerThanLump("Globaldata", filename);
		}
		globalData = new float[header.globaldata_num];
		for(int i = 0; i < header.globaldata_num; ++i)
		{
			globalData[i] = READ_FLOAT(vmData, header.globaldata_offset + i * 4);
		}
		
		for(int i = 0; i < header.globaldefs_num; ++i)
		{
			if(globalDefs[i].nameOffset < 0 || globalDefs[i].nameOffset >= header.stringdata_size)
			{
				throw new Error.OffsetOutOfBounds("Globaldef", i, "name", filename);
			}
			if(globalDefs[i].offset < 0 || globalDefs[i].offset >= header.entity_size)
			{
				throw new Error.OffsetOutOfBounds("Globaldef", i, filename);
			}
		}
		
		for(int i = 0; i < header.fielddefs_num; ++i)
		{
			if(fieldDefs[i].nameOffset < 0 || fieldDefs[i].nameOffset >= header.stringdata_size)
			{
				throw new Error.OffsetOutOfBounds("Fielddef", i, "name", filename);
			}
			if(fieldDefs[i].offset < 0 || fieldDefs[i].offset >= header.entity_size)
			{
				throw new Error.OffsetOutOfBounds("Fielddef", i, filename);
			}
		}
		
		for(int i = 0; i < header.functions_num; ++i)
		{
			if(functions[i].offsetFirstStatement >= header.statements_num)
			{
				throw new Error.OffsetOutOfBounds("Function", i, "first statement", filename);
			}
			if(functions[i].offsetLocalsInGlobals < 0 || functions[i].offsetLocalsInGlobals + functions[i].numLocals >= header.globaldata_num)
			{
				throw new Error.OffsetOutOfBounds("Function", i, "local parameters", filename);
			}
			if(functions[i].nameOffset < 0 || functions[i].nameOffset >= header.stringdata_size)
			{
				throw new Error.OffsetOutOfBounds("Function", i, "name", filename);
			}
			if(functions[i].fileNameOffset < 0 || functions[i].fileNameOffset >= header.stringdata_size)
			{
				throw new Error.OffsetOutOfBounds("Function", i, "filename", filename);
			}
		}
		
		for(int i = 0; i < header.statements_num; ++i)
		{
			if(statements[i].instruction < Instructions.MIN || statements[i].instruction > Instructions.MAX)
			{
				throw new Error.InvalidInstruction(i, statements[i].instruction, filename);
			}
			int offset;
			switch(statements[i].instruction)
			{
				case Instructions.DONE:
					break;
				case Instructions.LOAD_F:
				case Instructions.LOAD_V:
				case Instructions.LOAD_S:
				case Instructions.LOAD_ENT:
				case Instructions.LOAD_FLD:
				case Instructions.LOAD_FNC:
				case Instructions.STORE_F:
				case Instructions.STORE_V:
				case Instructions.STORE_S:
				case Instructions.STORE_ENT:
				case Instructions.STORE_FLD:
				case Instructions.STORE_FNC:
				case Instructions.STOREP_F:
				case Instructions.STOREP_V:
				case Instructions.STOREP_S:
				case Instructions.STOREP_ENT:
				case Instructions.STOREP_FLD:
				case Instructions.STOREP_FNC:
					BOUNDS_CHECK_GLOBAL(i, 0);
					BOUNDS_CHECK_GLOBAL(i, 1);
					break;
				case Instructions.NOT_F:
				case Instructions.NOT_V:
				case Instructions.NOT_S:
				case Instructions.NOT_ENT:
				case Instructions.NOT_FNC:
				case Instructions.STATE:
					BOUNDS_CHECK_GLOBAL(i, 0);
					BOUNDS_CHECK_GLOBAL(i, 2);
					break;
				case Instructions.IF:
				case Instructions.IFNOT:
					BOUNDS_CHECK_GLOBAL(i, 0);
					offset = i + statements[i].parameter[1];
					if(offset < 0 || offset >= header.statements_num)
					{
						throw new Error.OffsetOutOfBounds("Instruction", i, "parameter 1", filename);
					}
					break;
				case Instructions.CALL0:
				case Instructions.CALL1:
				case Instructions.CALL2:
				case Instructions.CALL3:
				case Instructions.CALL4:
				case Instructions.CALL5:
				case Instructions.CALL6:
				case Instructions.CALL7:
				case Instructions.CALL8:
				case Instructions.RETURN:
					BOUNDS_CHECK_GLOBAL(i, 0);
					break;
				case Instructions.GOTO:
					offset = i + statements[i].parameter[0];
					if(offset < 0 || offset >= header.statements_num)
					{
						throw new Error.OffsetOutOfBounds("Instruction", i, "parameter 0", filename);
					}
					break;
				default:
					BOUNDS_CHECK_GLOBAL(i, 0);
					BOUNDS_CHECK_GLOBAL(i, 1);
					BOUNDS_CHECK_GLOBAL(i, 2);
					break;
			}
		}
		stringData[header.stringdata_size-1] = '\0';
		statements[header.statements_num-1].instruction = Instructions.DONE;

		globalDefData = new byte[header.globaldefs_num];
		boolean[] isLocal = new boolean[header.globaldata_num];
		for(int i = 0; i < header.globaldata_num; ++i)
		{
			isLocal[i] = false;
		}
		for(int i = 0; i < header.functions_num; ++i)
		{
			for(int j = 0; j < functions[i].numLocals; ++j)
			{
				isLocal[functions[i].offsetLocalsInGlobals + j] = true;
			}
		}
		
		boolean end_sys = false;
		for(int i = 0; i < header.globaldata_num; ++i)
		{
			if(!end_sys)
			{
				globalDefData[i] = GLOBAL_DEF_SYSTEM;
				if(!"end_sys_globals".equals(nameForGlobalOffset(i)))
				{
					globalDefData[i] |= GLOBAL_DEF_SPECIAL;
				}
				if(!"end_sys_fields".equals(nameForGlobalOffset(i)))
				{
					globalDefData[i] |= GLOBAL_DEF_SPECIAL;
					end_sys = true;
				}
				if(isLocal[globalDefs[i].offset])
				{
					globalDefData[i] |= GLOBAL_DEF_LOCAL;
				}
			}
		}
		fieldOffsetTypes = new DefinitionType[header.entity_size];
		for(int i = 0; i < header.entity_size; ++i)
		{
			fieldOffsetTypes[i] = new DefinitionType(DefinitionType.NOTYPE);
		}
		for(int i = 0; i < header.fielddefs_num; ++i)
		{
			DefinitionType type = new DefinitionType(fieldDefs[i].type);
			short offset = fieldDefs[i].offset;
			if(fieldOffsetTypes[i].t != DefinitionType.VECTOR)
			{
				fieldOffsetTypes[i] = type;
			}
		}
		
		entityManager.init(header.entity_size, ENTITY_REUSE_DELAY);
		stringManager.init(stringData/*, header.stringdata_size*/);
	}
	
	public void removeBuiltin(int num)
	{
		builtins.remove(-num);
	}
	
	public void addBuiltin(BuiltinCallback builtin, int num)
	{
		removeBuiltin(num);
		builtins.put(-num, builtin);
	}
	
	public int findBuiltinNum(String name)
	{
		for(int i = 0; i < header.functions_num; ++i)
		{
			if(functions[i].offsetFirstStatement < 0)
			{
				if(name.equals(stringManager.getString(functions[i].nameOffset)))
				{
					return -functions[i].offsetFirstStatement;
				}
			}
		}
		return 0;
	}
	
	public boolean runBuiltin(int num)
	{
		if(builtins.get(-num) == null)
		{
			throw new Error.BuiltinNotFound(-num);
		}
		return builtins.get(-num).callback(this, num);
	}
	
	private void COPY_VEC(float[] a, float[] b)
	{
		b[0] = a[0];
		b[1] = a[1];
		b[2] = a[2];
	}
	
	private String GET_STRING(int offset)
	{
		return stringManager.getString(offset);
	}
	
	boolean runFunction(int functionNum, Integer instructionCount)
	{
		if(functionNum <= 0 || functionNum >= header.functions_num)
		{
			throw new Error.InvalidFunctionIndex(functionNum);
		}
		
		Function function = functions[functionNum];
		
		if(function.offsetFirstStatement < 0)
		{
			return runBuiltin(-function.offsetFirstStatement);
		}
		
		float[] stackData = new float[function.numLocals];
		for(int i = 0; i < function.numLocals; ++i)
		{
			stackData[i] = globalData[function.offsetLocalsInGlobals + i];
		}
		
		for(int i = 0, ofs = 0; i < function.numParameters; ++i)
		{
			for(int j = 0; j < function.parameterSizes[i]; ++j, ++ofs)
			{
				globalData[function.offsetLocalsInGlobals + ofs] = globalData[OFS_PARM0 + i*3 + j];
			}
		}
		//int32_t *intGlobalData = (int32_t*)mGlobalData;
		int currentStatement = function.offsetFirstStatement;
		
		int stopcode;
		final int STOP_SUCCESS			= 1;
		final int STOP_ERROR_HANDLED_ALREADY	= -1;
		final int STOP_ERROR_ENTITY_READ	= -2;
		final int STOP_ERROR_ENTITY_WRITE	= -3;
		final int STOP_ERROR_RUNAWAY_LOOP	= -4;
		for(stopcode = 0; stopcode == 0; ++instructionCount, ++currentStatement)
		{
			if((instructionCount & 0xffe00000) != 0)
			{
				stopcode = STOP_ERROR_RUNAWAY_LOOP;
				//goto end_of_instructions;
			}
			switch(statements[currentStatement].instruction)
			{
				case Instructions.DONE:
				case Instructions.RETURN:
					//COPY_VEC(globalData[statements[currentStatement].parameter], globalData[OFS_RETURN]);
					stopcode = STOP_SUCCESS;
					break;
				case Instructions.MUL_F:
					globalData[statements[currentStatement].parameter[2]] = globalData[statements[currentStatement].parameter[0]] * globalData[statements[currentStatement].parameter[1]];
			}
		}
	}
}
