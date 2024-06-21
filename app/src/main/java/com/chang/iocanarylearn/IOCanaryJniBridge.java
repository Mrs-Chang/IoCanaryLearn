package com.chang.iocanarylearn;

import java.util.ArrayList;

public class IOCanaryJniBridge {

	private static final class JavaContext {
		private final String stack;
		private String threadName;

		private JavaContext() {
			//stack = IOCanaryUtil.getThrowableStack(new Throwable());
			stack = "";
			if (null != Thread.currentThread()) {
				threadName = Thread.currentThread().getName();
			}
		}
	}


	public static void install() {

	}

	private static boolean loadJni() {
//		if (sIsLoadJniLib) {
//			return true;
//		}

		try {
			System.loadLibrary("io-canary");
		} catch (Exception e) {
//			MatrixLog.e(TAG, "hook: e: %s", e.getLocalizedMessage());
//			sIsLoadJniLib = false;
			return false;
		}

		//sIsLoadJniLib = true;
		return true;
	}

	public static void onIssuePublish(ArrayList<IOIssue> issues) {
//		if (sOnIssuePublishListener == null) {
//			return;
//		}
//
//		sOnIssuePublishListener.onIssuePublish(issues);
	}

	private static JavaContext getJavaContext() {
		try {
			return new JavaContext();
		} catch (Throwable th) {
//			MatrixLog.printErrStackTrace(TAG, th, "get javacontext exception");
		}

		return null;
	}

	private static native void enableDetector(int detectorType);

	private static native void setConfig(int key, long val);

	private static native boolean doHook();

	private static native boolean doUnHook();

}
