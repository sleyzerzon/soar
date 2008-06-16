/* Generated By:JJTree: Do not edit this line. SimpleNode.java */
/* JJT: 0.3pre1 */

import java.io.PrintWriter;

public class SimpleNode implements Node {
  protected Node parent;
  protected Node[] children;
  protected int id;

  public SimpleNode(int i) {
    id = i;
  }

  public void jjtOpen() {
  }

  public void jjtClose() {
  }
  
  public void jjtSetParent(Node n) { parent = n; }
  public Node jjtGetParent() { return parent; }

  public void jjtAddChild(Node n, int i) {
    if (children == null) {
      children = new Node[i + 1];
    } else if (i >= children.length) {
      Node c[] = new Node[i + 1];
      System.arraycopy(children, 0, c, 0, children.length);
      children = c;
    }
    children[i] = n;
  }

  public Node jjtGetChild(int i) {
    return children[i];
  }

  public int jjtGetNumChildren() {
    return (children == null) ? 0 : children.length;
  }

  /* You can override these two methods in subclasses of SimpleNode to
     customize the way the node appears when the tree is dumped.  If
     your output uses more than one line you should override
     toString(String), otherwise overriding toString() is probably all
     you need to do. */

  public String toString() { return ToyParserTreeConstants.jjtNodeName[id]; }
  public String toString(String prefix) { return prefix + toString(); }

  /* Override this method if you want to customize how the node dumps
     out its children. */

  public void dump(String prefix) {
    System.out.println(toString(prefix));
    if (children != null) {
      for (int i = 0; i < children.length; ++i) {
	SimpleNode n = (SimpleNode)children[i];
	if (n != null) {
	  n.dump(prefix + " ");
	}
      }
    }
  }

// Manually inserted code begins here

  protected Token begin, end;
  public void setFirstToken(Token t) { begin = t; }
  public void setLastToken(Token t) { end = t; }

  public void process (PrintWriter ostr) {
    System.out.println("Error - this should not be called");
    throw new Error();
  }

  // The following method prints token t, as well as all preceding 
  // special tokens (essentially, white space and comments).

  protected void print(Token t, PrintWriter ostr) {
    Token tt = t.specialToken;
    if (tt != null) {
      while (tt.specialToken != null) tt = tt.specialToken;
      while (tt != null) {
        ostr.print(addUnicodeEscapes(tt.image));
        tt = tt.next;
      }
    }
    ostr.print(addUnicodeEscapes(t.image));
  }

  private String addUnicodeEscapes(String str) {
    String retval = "";
    char ch;
    for (int i = 0; i < str.length(); i++) {
      ch = str.charAt(i);
      if ((ch < 0x20 || ch > 0x7e) && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\f') {
  	String s = "0000" + Integer.toString(ch, 16);
  	retval += "\\u" + s.substring(s.length() - 4, s.length());
      } else {
        retval += ch;
      }
    }
    return retval;
  }
}

