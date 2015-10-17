package com.twicestruck.jqc.vm;

public class Function
{
	public int offsetFirstStatement;
	public int offsetLocalsInGlobals;
	public int numLocals; // all parameters + locals
	public int profiling; // used at runtime
	public int nameOffset;
	public int fileNameOffset;
	public int numParameters;
	public byte[] parameterSizes = new byte[8]; // 1 or 3
	public static int SIZEOF = 4*7 + 8;
}
