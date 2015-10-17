package com.twicestruck.jqc.vm;

@SuppressWarnings("serial")
public abstract class Error extends RuntimeException
{
	public Error(String message)
	{
		super(message);
	}

	public static class OffsetOutOfBounds extends Error
	{
		public OffsetOutOfBounds(String def, int offset, String filename)
		{
			super(def + " " + offset + " is out of bounds in " + filename);
		} 
		public OffsetOutOfBounds(String def, int offset, String offsetType, String filename)
		{
			super(def + " " + offset + " " + offsetType + " is out of bounds in " + filename);
		} 
	}

	public static class ProgsSmallerThanHeader extends Error
	{
		public ProgsSmallerThanHeader(int vmSize, int headerSize)
		{
			super("VM size " + vmSize + " can't be smaller than header size " + headerSize);
		}
	}
	
	public static class ProgsInvalidVersion extends Error
	{
		public ProgsInvalidVersion(int version, int required)
		{
			super("Progs version " + version + " does not match required version " + required);
		}
	}
	
	public static class ProgsSmallerThanLump extends Error
	{
		public ProgsSmallerThanLump(String lumpName, String filename)
		{
			super(lumpName + " lump exceeds total progs size in " + filename);
		}
	}
	
	public static class InvalidInstruction extends Error
	{
		public InvalidInstruction(int num, short instruction, String filename)
		{
			super("Instruction #" + num + "(" + Integer.toHexString(instruction) + ") is invalid in " + filename);
		}
	}
	
	public static class BuiltinNotFound extends Error
	{
		public BuiltinNotFound(int num)
		{
			super("Builtin #" + num + " not found, did you forget to register it?");
		}
	}
	
	public static class InvalidFunctionIndex extends Error
	{
		public InvalidFunctionIndex(int num)
		{
			super("Function index " + num + " is invalid");
		}
		
	}
}
