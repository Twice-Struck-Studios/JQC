package com.twicestruck.jqc.vm;

public class DefinitionType
{
	public static final int NOTYPE   = 0;
	public static final int STRING   = 1;
	public static final int FLOAT    = 2;
	public static final int VECTOR   = 3;
	public static final int ENTITY   = 4;
	public static final int FIELD    = 5;
	public static final int FUNCTION = 6;
	public int t;
	
	public DefinitionType(int t)
	{
		this.t = t;
	}
}
