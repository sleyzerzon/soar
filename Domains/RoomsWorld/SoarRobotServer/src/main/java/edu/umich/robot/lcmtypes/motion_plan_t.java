/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package edu.umich.robot.lcmtypes;
 
import java.io.*;
import java.nio.*;
import java.util.*;
import lcm.lcm.*;
 
public final class motion_plan_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public int nsteps;
    public double xy[][];
 
    public motion_plan_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x99fc04092f2af0b3L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(motion_plan_t.class))
            return 0L;
 
        classes.add(motion_plan_t.class);
        long hash = LCM_FINGERPRINT_BASE
            ;
        classes.remove(classes.size() - 1);
        return (hash<<1) + ((hash>>63)&1);
    }
 
    public void encode(DataOutput outs) throws IOException
    {
        outs.writeLong(LCM_FINGERPRINT);
        _encodeRecursive(outs);
    }
 
    public void _encodeRecursive(DataOutput outs) throws IOException
    {
        outs.writeLong(this.utime); 
 
        outs.writeInt(this.nsteps); 
 
        for (int a = 0; a < this.nsteps; a++) {
            for (int b = 0; b < 2; b++) {
                outs.writeDouble(this.xy[a][b]); 
            }
        }
 
    }
 
    public motion_plan_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public motion_plan_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static motion_plan_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        motion_plan_t o = new motion_plan_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.utime = ins.readLong();
 
        this.nsteps = ins.readInt();
 
        this.xy = new double[(int) nsteps][(int) 2];
        for (int a = 0; a < this.nsteps; a++) {
            for (int b = 0; b < 2; b++) {
                this.xy[a][b] = ins.readDouble();
            }
        }
 
    }
 
    public motion_plan_t copy()
    {
        motion_plan_t outobj = new motion_plan_t();
        outobj.utime = this.utime;
 
        outobj.nsteps = this.nsteps;
 
        outobj.xy = new double[(int) nsteps][(int) 2];
        for (int a = 0; a < this.nsteps; a++) {
            System.arraycopy(this.xy[a], 0, outobj.xy[a], 0, 2);        }
 
        return outobj;
    }
 
}

