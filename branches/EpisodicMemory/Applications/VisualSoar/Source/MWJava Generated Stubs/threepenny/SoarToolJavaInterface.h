/* DO NOT EDIT THIS FILE - it is machine generated */
/* Generated for class threepenny.SoarToolJavaInterface */
/* Generated from SoarToolJavaInterface.java*/

#ifndef _Included_threepenny_SoarToolJavaInterface
#define _Included_threepenny_SoarToolJavaInterface
#include <jni.h>
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_InitInterfaceLibrary
 * Signature: (Ljava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1InitInterfaceLibrary
	(JNIEnv *, jobject, jstring, jboolean);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_InitListenPort
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1InitListenPort
	(JNIEnv *, jobject, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_EstablishConnections
 * Signature: (ILjava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1EstablishConnections
	(JNIEnv *, jobject, jint, jstring, jboolean);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_GetNumberConnections
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1GetNumberConnections
	(JNIEnv *, jobject, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_GetConnectionName
 * Signature: (II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1GetConnectionName
	(JNIEnv *, jobject, jint, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_EnableConnectionByIndex
 * Signature: (IIZ)V
 */
JNIEXPORT void JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1EnableConnectionByIndex
	(JNIEnv *, jobject, jint, jint, jboolean);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_EnableConnectionByName
 * Signature: (ILjava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1EnableConnectionByName
	(JNIEnv *, jobject, jint, jstring, jboolean);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_IsConnectionEnabledByIndex
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1IsConnectionEnabledByIndex
	(JNIEnv *, jobject, jint, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_IsConnectionEnabledByName
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1IsConnectionEnabledByName
	(JNIEnv *, jobject, jint, jstring);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_TerminateInterfaceLibrary
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1TerminateInterfaceLibrary
	(JNIEnv *, jobject, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_PumpMessages
 * Signature: (IZ)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1PumpMessages
	(JNIEnv *, jobject, jint, jboolean);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_IsIncomingCommandAvailable
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1IsIncomingCommandAvailable
	(JNIEnv *, jobject, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_GetIncomingCommand
 * Signature: (ILthreepenny/SoarToolJavaCommand;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1GetIncomingCommand
	(JNIEnv *, jobject, jint, jobject);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_PopIncomingCommand
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1PopIncomingCommand
	(JNIEnv *, jobject, jint);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendCommand
 * Signature: (IZLthreepenny/SoarToolJavaCommand;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendCommand
	(JNIEnv *, jobject, jint, jboolean, jobject);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendProduction
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendProduction
	(JNIEnv *, jobject, jint, jstring, jstring);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendFile
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendFile
	(JNIEnv *, jobject, jint, jstring);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendProductionMatches
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendProductionMatches
	(JNIEnv *, jobject, jint, jstring);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendExciseProduction
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendExciseProduction
	(JNIEnv *, jobject, jint, jstring);

/*
 * Class:     threepenny_SoarToolJavaInterface
 * Method:    jniSTI_SendRawCommand
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_threepenny_SoarToolJavaInterface_jniSTI_1SendRawCommand
	(JNIEnv *, jobject, jint, jstring);

#ifdef __cplusplus
}
#endif
#endif
