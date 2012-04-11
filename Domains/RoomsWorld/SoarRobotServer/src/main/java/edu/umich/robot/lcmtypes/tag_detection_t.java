/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package edu.umich.robot.lcmtypes;
 
import java.io.*;
import java.nio.*;
import java.util.*;
import lcm.lcm.*;
 
public final class tag_detection_t implements lcm.lcm.LCMEncodable
{
    public int id;
    public int errors;
    public double homography[][];
    public double hxy[];
 
    public tag_detection_t()
    {
        homography = new double[3][3];
        hxy = new double[2];
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x1e412523c59902d8L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(april.lcmtypes.tag_detection_t.class))
            return 0L;
 
        classes.add(april.lcmtypes.tag_detection_t.class);
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
        outs.writeInt(this.id); 
 
        outs.writeInt(this.errors); 
 
        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                outs.writeDouble(this.homography[a][b]); 
            }
        }
 
        for (int a = 0; a < 2; a++) {
            outs.writeDouble(this.hxy[a]); 
        }
 
    }
 
    public tag_detection_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public tag_detection_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static april.lcmtypes.tag_detection_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        april.lcmtypes.tag_detection_t o = new april.lcmtypes.tag_detection_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.id = ins.readInt();
 
        this.errors = ins.readInt();
 
        this.homography = new double[(int) 3][(int) 3];
        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                this.homography[a][b] = ins.readDouble();
            }
        }
 
        this.hxy = new double[(int) 2];
        for (int a = 0; a < 2; a++) {
            this.hxy[a] = ins.readDouble();
        }
 
    }
 
    public april.lcmtypes.tag_detection_t copy()
    {
        april.lcmtypes.tag_detection_t outobj = new april.lcmtypes.tag_detection_t();
        outobj.id = this.id;
 
        outobj.errors = this.errors;
 
        outobj.homography = new double[(int) 3][(int) 3];
        for (int a = 0; a < 3; a++) {
            System.arraycopy(this.homography[a], 0, outobj.homography[a], 0, 3);        }
 
        outobj.hxy = new double[(int) 2];
        System.arraycopy(this.hxy, 0, outobj.hxy, 0, 2); 
        return outobj;
    }
 
}

