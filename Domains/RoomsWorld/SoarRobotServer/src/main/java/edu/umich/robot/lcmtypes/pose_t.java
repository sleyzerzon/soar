/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package edu.umich.robot.lcmtypes;
 
import java.io.*;
import java.nio.*;
import java.util.*;
import lcm.lcm.*;
 
public final class pose_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public double pos[];
    public double vel[];
    public double orientation[];
    public double rotation_rate[];
    public double accel[];
 
    public pose_t()
    {
        pos = new double[3];
        vel = new double[3];
        orientation = new double[4];
        rotation_rate = new double[3];
        accel = new double[3];
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x170b77d82958082fL;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(april.lcmtypes.pose_t.class))
            return 0L;
 
        classes.add(april.lcmtypes.pose_t.class);
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
 
        for (int a = 0; a < 3; a++) {
            outs.writeDouble(this.pos[a]); 
        }
 
        for (int a = 0; a < 3; a++) {
            outs.writeDouble(this.vel[a]); 
        }
 
        for (int a = 0; a < 4; a++) {
            outs.writeDouble(this.orientation[a]); 
        }
 
        for (int a = 0; a < 3; a++) {
            outs.writeDouble(this.rotation_rate[a]); 
        }
 
        for (int a = 0; a < 3; a++) {
            outs.writeDouble(this.accel[a]); 
        }
 
    }
 
    public pose_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public pose_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static april.lcmtypes.pose_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        april.lcmtypes.pose_t o = new april.lcmtypes.pose_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.utime = ins.readLong();
 
        this.pos = new double[(int) 3];
        for (int a = 0; a < 3; a++) {
            this.pos[a] = ins.readDouble();
        }
 
        this.vel = new double[(int) 3];
        for (int a = 0; a < 3; a++) {
            this.vel[a] = ins.readDouble();
        }
 
        this.orientation = new double[(int) 4];
        for (int a = 0; a < 4; a++) {
            this.orientation[a] = ins.readDouble();
        }
 
        this.rotation_rate = new double[(int) 3];
        for (int a = 0; a < 3; a++) {
            this.rotation_rate[a] = ins.readDouble();
        }
 
        this.accel = new double[(int) 3];
        for (int a = 0; a < 3; a++) {
            this.accel[a] = ins.readDouble();
        }
 
    }
 
    public april.lcmtypes.pose_t copy()
    {
        april.lcmtypes.pose_t outobj = new april.lcmtypes.pose_t();
        outobj.utime = this.utime;
 
        outobj.pos = new double[(int) 3];
        System.arraycopy(this.pos, 0, outobj.pos, 0, 3); 
        outobj.vel = new double[(int) 3];
        System.arraycopy(this.vel, 0, outobj.vel, 0, 3); 
        outobj.orientation = new double[(int) 4];
        System.arraycopy(this.orientation, 0, outobj.orientation, 0, 4); 
        outobj.rotation_rate = new double[(int) 3];
        System.arraycopy(this.rotation_rate, 0, outobj.rotation_rate, 0, 3); 
        outobj.accel = new double[(int) 3];
        System.arraycopy(this.accel, 0, outobj.accel, 0, 3); 
        return outobj;
    }
 
}

