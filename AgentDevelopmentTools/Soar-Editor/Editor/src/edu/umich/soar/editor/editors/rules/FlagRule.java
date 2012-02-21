/*
 *Copyright (c) 2009, Soar Technology, Inc.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without modification,   *are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Soar Technology, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY  *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED   *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.   *IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,   *INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT   *NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR   *PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,    *WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE    *POSSIBILITY OF SUCH *DAMAGE. 
 *
 * 
 */
package edu.umich.soar.editor.editors.rules;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;

import edu.umich.soar.editor.editors.SyntaxColorManager;

/**
 * <code>FlagRule</code> <code>WordRule</code> implementation for Soar
 * rule flags.
 *
 * @author annmarie.steichmann@soartech.com
 * @version $Revision: 578 $ $Date: 2009-06-22 13:05:30 -0400 (Mon, 22 Jun 2009) $
 */
public class FlagRule
    extends KeywordRule {
    
    private static String[] RULE_FLAGS = new String[] {
        "o-support", 
        "i-support", 
        "default", 
        "chunk",
        "monitor",
        "interrupt",
        "template"
    };
    
    private static String[][] ALL_FLAGS = new String[][]{ RULE_FLAGS };    
    
    /**
     * Constructor for a <code>FlagRule</code> object.
     */
    public FlagRule() {
        
        super( new FlagDetector() );
    }

    /* (non-Javadoc)
     * @see com.soartech.soar.ide.ui.editors.text.rules.KeywordRule#getFgColor()
     */
    @Override
    protected Color getFgColor() {

        return SyntaxColorManager.getFlagColor();
    }

    /* (non-Javadoc)
     * @see com.soartech.soar.ide.ui.editors.text.rules.KeywordRule#getBgColor()
     */
    @Override
    protected Color getBgColor() {

        return null;
    }

    /* (non-Javadoc)
     * @see com.soartech.soar.ide.ui.editors.text.rules.KeywordRule#getProperty()
     */
    @Override
    protected int getProperty() {

        return SWT.BOLD;
    }

    /* (non-Javadoc)
     * @see com.soartech.soar.ide.ui.editors.text.rules.KeywordRule#getAllKeywords()
     */
    @Override
    protected String[][] getAllKeywords() {

        return ALL_FLAGS;
    }
}
