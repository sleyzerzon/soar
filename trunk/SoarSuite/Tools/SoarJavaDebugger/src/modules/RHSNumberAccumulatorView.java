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

public class RHSNumberAccumulatorView extends RHSFunTextView
{
	public String getModuleBaseName() { return "rhs_number_accumulator" ; }
	
	@Override
	protected void updateNow() {
		setTextSafely(Double.toString(totalValue));
	}

	double totalValue = 0;

	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		if (functionName.equals("--clear")) {
			clear = true;
			return debugMessages ? m_Name + ":" + functionName + ": set to clear" : "";
		}
		
		double value = 0;
		try {
			value = Double.parseDouble(argument);
		} catch (NumberFormatException e) {
			return m_Name + ":" + functionName + ": Unknown argument to " + rhsFunName;
		}
		
		totalValue += value;
		
		return debugMessages ? m_Name + ":" + functionName + ": Total value changed to: " + totalValue : "";
	}
	
	@Override
	public void onInitSoar() {
		if (clear) {
			this.onInitSoar();
		}
		
		totalValue = 0;
		updateNow();
	}
}
