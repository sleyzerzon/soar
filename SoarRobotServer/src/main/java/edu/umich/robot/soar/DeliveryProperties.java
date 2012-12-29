/*
 * Copyright (c) 2011, Regents of the University of Michigan
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package edu.umich.robot.soar;

import edu.umich.robot.util.properties.PropertyKey;

/**
 * @author justinnhli@gmail.com
 */
public class DeliveryProperties
{
    public static final PropertyKey<String> METHOD_ECOLOGICAL_OBJECTS = 
        PropertyKey.builder("method-ecological-objects", String.class)
        .defaultValue("no")
        .build();

    public static final PropertyKey<String> METHOD_ECOLOGICAL_TIMING = 
        PropertyKey.builder("method-ecological-timing", String.class)
        .defaultValue("no")
        .build();

    public static final PropertyKey<Integer> METHOD_ECOLOGICAL_TIMING_INTERVAL = 
        PropertyKey.builder("method-ecological-timing-interval", Integer.class)
        .defaultValue(1000)
        .build();

    public static final PropertyKey<String> METHOD_ECOLOGICAL_DOORS = 
        PropertyKey.builder("method-ecological-doors", String.class)
        .defaultValue("no")
        .build();

    public static final PropertyKey<String> METHOD_ECOLOGICAL_ENTRY = 
        PropertyKey.builder("method-ecological-entry", String.class)
        .defaultValue("no")
        .build();

    public static final PropertyKey<String> TASKS_HELD_IN = 
        PropertyKey.builder("tasks-held-in", String.class) 
        .defaultValue("wm") // acceptable values: wm smem
        .build();

    public static final PropertyKey<Double> DECAY_RATE = 
        PropertyKey.builder("decay-rate", Double.class)
        .defaultValue(0.0)
        .build();

    public static final PropertyKey<Boolean> F = 
        PropertyKey.builder("epmem-learning", Boolean.class) 
        .defaultValue(Boolean.FALSE) 
        .build();

}
