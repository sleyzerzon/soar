#ifndef CLI_CLIERROR_H
#define CLI_CLIERROR_H

namespace cli {

	typedef int ErrorCode;

	class CLIError {
	public:
		enum {
			kNoError							= 0,
			//kCustomError						= 1,
			kgSKIError							= 2,
			kGetOptError						= 3,
			kCommandNotFound					= 4,
			kProductionNotFound					= 5,
			kMultiAttributeNotFound				= 6,
			kNoCommandPointer					= 7,
			kNotImplemented						= 8,
			kOptionNotImplemented				= 9,
			kExtraClosingParen					= 10,
			kUnmatchedBracketOrQuote			= 11,
			kExtraClosingBrace 					= 12,
			kUnmatchedBrace 					= 13,
			kTooManyArgs 						= 14,
			kTooFewArgs 						= 15,
			kUnrecognizedOption 				= 16,
			kMissingOptionArg 					= 17,
			kgetcwdFail 						= 18,
			kgettimeofdayFail 					= 19,
			kchdirFail							= 20,
			kAgentRequired						= 21,
			kKernelRequired						= 22,
			kAliasNotFound						= 23,
			kAliasExists						= 24,
			//kAliasError						= 25,
			kNoUsageInfo						= 26,
			kNoUsageFile						= 27,
			kIntegerExpected					= 28,
			kIntegerMustBePositive				= 29,
			kIntegerMustBeNonNegative			= 30,
			kIntegerOutOfRange					= 31,
			kInvalidOperation					= 32,
			kInvalidWMEDetail					= 33,
			kInvalidNumericIndifferentMode		= 34,
			kInvalidIndifferentSelectionMode	= 35,
			kInvalidProductionType				= 36,
			kNoProdTypeWhenProdName				= 37,
			kSourceOnlyOneFile					= 38,
			kLogAlreadyOpen						= 39,
			kLogOpenFailure						= 40,
			kLogNotOpen							= 41,
			kDirectoryOpenFailure				= 42,
			kDirectoryEntryReadFailure			= 43,
			kDirectoryStackEmpty				= 44,
			kMissingFilenameArg					= 45,
			kOpenFileFail						= 46,
			kCantSaveReteWithJustifications		= 47,
			kCantLoadReteWithProductions		= 48,
			kReteSaveOperationFail				= 49,
			kReteLoadOperationFail				= 50,
			kInvalidProduction					= 51,
			kInvalidLearnSetting				= 52,
			kRemoveOrZeroExpected				= 53,
			kInvalidID							= 54,
			kInvalidAttribute					= 55,
			kInvalidValue						= 56,
			kRemoveWMEFailed					= 57,
			kInvalidOSupportMode				= 58,
			kInvalidWMEFilterType				= 59,
			kFilterExpected						= 60,
			kDuplicateWMEFilter					= 61, 
			kInvalidMode						= 62,
			kTypeRequired						= 63,
			kWMEFilterNotFound					= 64,
			kProductionRequired					= 65,
			kInvalidConditionNumber				= 66,
			kInvalidPrefix						= 67,
			kCountGreaterThanMaxChunks			= 68,
			kCountLessThanChunks				= 69,
			kAcceptableOrNothingExpected		= 70,
			kSourceError						= 71,
		};

		static char const* GetErrorDescription(ErrorCode code) {
			switch (code) {
				case kNoError:							return "No Error.";
				//case kCustomError:						return m_Description.c_str();
				case kgSKIError:						return "gSKI error.";
				case kGetOptError:						return "GetOpt returned with an error.";
				case kCommandNotFound:					return "Command not found.";
				case kProductionNotFound:				return "Production not found.";
				case kMultiAttributeNotFound:			return "Multi-attribute not found.";
				case kNoCommandPointer:					return "Command found but function pointer is NULL.";
				case kNotImplemented:					return "Not implemented.";
				case kOptionNotImplemented:				return "Option not implemented.";
				case kExtraClosingParen:				return "Closing bracket found without opening counterpart.";
				case kUnmatchedBracketOrQuote:			return "No closing quotes/brackets found.";
				case kExtraClosingBrace:				return "Closing brace found without opening counterpart.";
				case kUnmatchedBrace:					return "Unexpected end of file. Unmatched opening brace.";
				case kTooManyArgs:						return "Too many arguments for the specified (or unspecified) options, check syntax.";
				case kTooFewArgs:						return "Too few arguments for the specified (or unspecified) options, check syntax.";
				case kUnrecognizedOption:				return "Unrecognized option.";
				case kMissingOptionArg:					return "Missing option argument.";
				case kgetcwdFail:						return "Error getting current working directory.";
				case kgettimeofdayFail:					return "gettimeofday() failed.";
				case kchdirFail:						return "Error changing to directory.";
				case kAgentRequired:					return "An agent is required for this command.";
				case kKernelRequired:					return "A kernel is required for this command.";
				case kAliasNotFound:					return "Alias not found.";
				case kAliasExists:						return "Alias exists, remove to overwrite.";
				//case kAliasError:						return "Error adding alias.";
				case kNoUsageInfo:						return "Usage information not found for the command.";
				case kNoUsageFile:						return "Usage file not available (error opening file).";
				case kIntegerExpected:					return "Integer argument expected.";
				case kIntegerMustBePositive:			return "Integer argument must be positive.";
				case kIntegerMustBeNonNegative:			return "Integer argument must be non-negative.";
				case kIntegerOutOfRange:				return "Integer argument out of range.";
				case kInvalidOperation:					return "Invalid operation.";
				case kInvalidWMEDetail:					return "Invalid WME detail level.";
				case kInvalidNumericIndifferentMode:	return "Invalid numeric indifferent mode.";
				case kInvalidIndifferentSelectionMode:	return "Invalid indifferent selection mode.";
				case kInvalidProductionType:			return "Invalid production type.";
				case kNoProdTypeWhenProdName:			return "Do not specify production type when specifying a production name.";
				case kSourceOnlyOneFile:				return "Source only one file at a time.";
				case kLogAlreadyOpen:					return "Log already open.";
				case kLogOpenFailure:					return "Failed to open file for logging.";
				case kLogNotOpen:						return "Log is not open.";
				case kDirectoryOpenFailure:				return "Unable to open directory for reading.";
				case kDirectoryEntryReadFailure:		return "Unable to read directory entry for reading.";
				case kDirectoryStackEmpty:				return "Directory stack empty, no directory to change to.";
				case kMissingFilenameArg:				return "Missing filename argument.";
				case kOpenFileFail:						return "Failed to open file for reading.";
				case kCantSaveReteWithJustifications:	return "Can't save rete while justifications are present.";
				case kCantLoadReteWithProductions:		return "Can't load rete unless production memory is empty.";
				case kReteSaveOperationFail:			return "Rete save operation failed.";
				case kReteLoadOperationFail:			return "Rete load operation failed.";
				case kInvalidProduction:				return "Invalid production.";
				case kInvalidLearnSetting:				return "Invalid learn setting, expected noprint, print, fullprint, or 0-2.";
				case kRemoveOrZeroExpected:				return "Invalid argument, expected remove or 0.";
				case kInvalidID:						return "Unknown or invalid ID.";
				case kInvalidAttribute:					return "Unknown or invalid attribute.";
				case kInvalidValue:						return "Unknown or invalid value.";
				case kRemoveWMEFailed:					return "Remove WME failed.";
				case kInvalidOSupportMode:				return "Invalid O-Support mode, use 0-4.";
				case kInvalidWMEFilterType:				return "Invalid WME filter type, expected 'adds' 'removes' or 'both'.";
				case kFilterExpected:					return "ID/Attribute/Value filter expected, one or more missing.";
				case kDuplicateWMEFilter:				return "That WME filter already exists.";
				case kInvalidMode:						return "Invalid mode.";
				case kTypeRequired:						return "A type (-t adds/removes/both) is required for this command.";
				case kWMEFilterNotFound:				return "The specified WME filter was not found.";
				case kProductionRequired:				return "A production is required.";
				case kInvalidConditionNumber:			return "Condition number must be a non-negative integer.";
				case kInvalidPrefix:					return "Failed to set prefix (does it contain a '*'?).";
				case kCountGreaterThanMaxChunks:		return "Cannot set count greater than the max-chunks sysparam.";
				case kCountLessThanChunks:				return "Cannot set chunk count less than the current number of chunks.";
				case kAcceptableOrNothingExpected:		return "Expected acceptable preference (+) or nothing, check syntax.";
				case kSourceError:						return "Source command error.";
				default:								return "Unknown error code.";
			}
		}
	}; // class
} // namespace 

#endif // CLI_CLIERROR_H
