/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.7
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.mapzen.tangram;

public final class DebugFlags {
    public final static DebugFlags freeze_tiles = new DebugFlags("freeze_tiles", 0);
    public final static DebugFlags proxy_colors = new DebugFlags("proxy_colors");
    public final static DebugFlags tile_bounds = new DebugFlags("tile_bounds");
    public final static DebugFlags tile_infos = new DebugFlags("tile_infos");
    public final static DebugFlags labels = new DebugFlags("labels");

    public final int swigValue() {
        return swigValue;
    }

    public String toString() {
        return swigName;
    }

    public static DebugFlags swigToEnum(int swigValue) {
        if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
            return swigValues[swigValue];
        for (int i = 0; i < swigValues.length; i++)
            if (swigValues[i].swigValue == swigValue)
                return swigValues[i];
        throw new IllegalArgumentException("No enum " + DebugFlags.class + " with value " + swigValue);
    }

    private DebugFlags(String swigName) {
        this.swigName = swigName;
        this.swigValue = swigNext++;
    }

    private DebugFlags(String swigName, int swigValue) {
        this.swigName = swigName;
        this.swigValue = swigValue;
        swigNext = swigValue+1;
    }

    private DebugFlags(String swigName, DebugFlags swigEnum) {
        this.swigName = swigName;
        this.swigValue = swigEnum.swigValue;
        swigNext = this.swigValue+1;
    }

    private static DebugFlags[] swigValues = { freeze_tiles, proxy_colors, tile_bounds, tile_infos, labels };
    private static int swigNext = 0;
    private final int swigValue;
    private final String swigName;
}

