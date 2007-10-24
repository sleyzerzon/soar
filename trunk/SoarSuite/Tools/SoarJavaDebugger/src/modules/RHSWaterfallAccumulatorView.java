package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.*;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;

import java.util.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

public class RHSWaterfallAccumulatorView extends RHSFunTextView implements Kernel.RhsFunctionInterface
{
	public String getModuleBaseName() { return "rhs_waterfall_accumulator" ; }
	
	@Override
	protected void updateNow() {
		if (currentTag == null) {
			this.clearDisplay();
			return;
		}
		
		if (newTag) {
			// rewrite entire text
			StringBuilder newText = new StringBuilder();
			newText.append(currentTag);
			newText.append(": ");
			newText.append(currentValue);
			newText.append("\n");
			newText.append(oldValues);

			setTextSafely(newText.toString());
			
		} else {
			// only change first line
			
			// If Soar is running in the UI thread we can make
			// the update directly.
			if (!Document.kDocInOwnThread)
			{
				changeCurrentValue();
				return ;
			}
			
			// Have to make update in the UI thread.
			// Callback comes in the document thread.
	        Display.getDefault().asyncExec(new Runnable() {
	            public void run() {
					changeCurrentValue();
	            }
	         }) ;
		}
	}
	
	private void changeCurrentValue() {
		
		// pull the string out of the widget
		String theText = textBox.toString();

		// find the first newline
		int index = theText.indexOf("\n");
		assert index != -1;

		// insert new stuff
		StringBuilder newText = new StringBuilder();
		newText.append(currentTag);
		newText.append(": ");
		newText.append(currentValue);
		newText.append("\n");
		
		textBox.replaceTextRange(0, index, newText.toString());
	}

	boolean newTag = false;
	String currentTag;
	double currentValue;
	StringBuilder oldValues = new StringBuilder();

	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		String[] commandLine = argument.split("\\s+");
		
		if (commandLine.length >= 1 && commandLine[0].equals("--clear")) {
			this.onInitSoar();
			return debugMessages ? m_Name + ":" + functionName + ": cleared" : "";
		}
		
		// make sure we have 2 args
		if (commandLine.length != 2) {
			return m_Name + ":" + functionName + ": Two arguments (tag, value) are required for RHS function " + rhsFunName + ", got " + commandLine.length + ".";
		}
		
		double value = 0;
		try {
			value = Double.parseDouble(commandLine[1]);
			
		} catch (NumberFormatException e) {
			return m_Name + ":" + functionName + ": expected number, got " + commandLine[1];
		}
		
		if (currentTag == null || !commandLine[0].equals(currentTag)) {
			newTag = true;
			
			if (currentTag != null) {
				// render old stuff
				oldValues.insert(0, "\n");
				oldValues.insert(0, currentValue);
				oldValues.insert(0, ": ");
				oldValues.insert(0, currentTag);
			}
			
			//create new tag
			currentTag = new String(commandLine[0]);
			
			// reset value
			currentValue = value;
		} else {
			// increment value
			currentValue += value;
		}
		
		return debugMessages ? m_Name + ":" + functionName + ": " + currentTag + ": " + currentValue : "";
	}
	
	@Override
	public void onInitSoar() {
		newTag = false;
		currentValue = 0;
		currentTag = null;
		oldValues = new StringBuilder();
		updateNow();
	}
}























