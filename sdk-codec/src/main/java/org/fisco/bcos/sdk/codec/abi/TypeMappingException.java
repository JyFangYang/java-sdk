package org.fisco.bcos.sdk.codec.abi;

public class TypeMappingException extends RuntimeException {

    public TypeMappingException(Exception e) {
        super(e);
    }

    public TypeMappingException(String message) {
        super(message);
    }

    public TypeMappingException(String message, Exception e) {
        super(message, e);
    }
}
